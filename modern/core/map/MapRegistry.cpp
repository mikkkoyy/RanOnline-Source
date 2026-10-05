#include "map/MapRegistry.h"

#include "navigation/WldNavigationLoader.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <system_error>

namespace Modern::Map
{
	namespace
	{
		// ------------------------------------------------------------------------
		// The deployed layout, and why discovery probes instead of assuming
		// ------------------------------------------------------------------------
		//
		// RAN's own subpaths are `\Data\GLogic\` (SUBPATH.cpp:30),
		// `\Data\GLogic\Level\` (:52) and the map directory under `\Data\Map\`. The
		// ASURA tree this milestone is proven against actually spells them
		// `data\glogic\` and `data\Map\`, so a single hardcoded spelling is one
		// repack away from failing. Each directory is therefore found by probing a
		// short candidate list - the same approach
		// `modern/tests/WldNavigationTests.cpp:91-99` already uses for `Data/Map`, so
		// this introduces no second convention.
		//
		// Once a directory IS found, file lookup inside it uses the name the asset
		// spells and lets the platform resolve the case. That is deliberate:
		//
		//   * `mapslist.mst` really does contain both `w_school_01.Lev` and
		//     `w_city_s_01.lev`, and RAN opens by concatenated path
		//     (`GetLevelPath() + strFile`, GLLevelFileSaveLoad.cpp:157-158).
		//   * Lowercasing before lookup would be MORE permissive than legacy and could
		//     silently open a file legacy would not have found.
		//
		// So the stored name is never rewritten. What the cache stores is the
		// path that was actually opened, canonicalised - which is how two maps naming
		// the same `.wld` still share one mesh.

		std::string LowerAscii(std::string value)
		{
			std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
				return static_cast<char>(std::tolower(c));
			});
			return value;
		}

		std::string FirstExistingDirectory(const std::filesystem::path& base,
		                                   std::initializer_list<const char*> relative)
		{
			for (const char* candidate : relative)
			{
				std::error_code ec;
				const std::filesystem::path path = base / candidate;
				if (std::filesystem::is_directory(path, ec))
				{
					return path.string();
				}
			}
			return std::string();
		}

		bool ReadWholeFile(const std::string& path, std::vector<std::uint8_t>& out)
		{
			std::error_code ec;
			const auto size = std::filesystem::file_size(path, ec);
			if (ec)
			{
				return false;
			}

			// A ceiling, so a corrupt or wrong file cannot ask for a huge allocation
			// before the first byte is read. `mapslist.mst` is 11,494 bytes and a
			// `.lev` is a few hundred KB; 64 MB is far above anything shipped and far
			// below anything that would hurt.
			constexpr std::uintmax_t kMaxAssetBytes = 64u * 1024u * 1024u;
			if (size > kMaxAssetBytes || size < 1)
			{
				return false;
			}

			std::ifstream file(path, std::ios::binary);
			if (!file)
			{
				return false;
			}

			out.resize(static_cast<std::size_t>(size));
			file.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(out.size()));
			return static_cast<std::size_t>(file.gcount()) == out.size();
		}

		// Resolves `name` inside `directory`, case-insensitively, without rewriting
		// the stored name.
		//
		// The direct open is tried FIRST and is the normal path: on Windows the
		// filesystem is case-insensitive, so `w_school_01.Lev` opens
		// `w_school_01.Lev` directly and nothing is scanned. The scan exists for a
		// case-sensitive filesystem, where it is the only way RAN's own concatenated
		// path would ever work - and it returns the name AS FOUND, which is what
		// gets cached and therefore what guarantees two spellings of one file share
		// one mesh.
		//
		// Returns an empty string when nothing matches.
		std::string ResolveFileCaseInsensitively(const std::string& directory,
		                                          const std::string& name)
		{
			if (name.empty())
			{
				return std::string();
			}

			const std::filesystem::path direct = std::filesystem::path(directory) / name;

			std::error_code ec;
			if (std::filesystem::is_regular_file(direct, ec))
			{
				return direct.string();
			}

			const std::string wanted = LowerAscii(name);

			ec.clear();
			std::filesystem::directory_iterator it(
			    std::filesystem::path(directory), std::filesystem::directory_options::skip_permission_denied,
			    ec);
			if (ec)
			{
				return std::string();
			}

			for (const auto& entry : it)
			{
				std::error_code entryEc;
				if (!entry.is_regular_file(entryEc))
				{
					continue;
				}
				if (LowerAscii(entry.path().filename().string()) == wanted)
				{
					return entry.path().string();
				}
			}

			return std::string();
		}

		// The identity of a `.wld` for caching purposes: the absolute, symlink-
		// resolved, case-folded path.
		//
		// Case FOLDING is deliberate. Without it, `Suhak.wld` and `suhak.wld` are
		// two cache keys on a filesystem where they are one file, and two map ids
		// would silently get two copies of the same mesh. It affects the cache key
		// ONLY - `MapEntry::wldFileName` keeps the spelling the asset used, because
		// that is what a log line and an operator need to see.
		std::string CanonicalCacheKey(const std::string& path)
		{
			std::error_code ec;
			std::filesystem::path canonical = std::filesystem::weakly_canonical(path, ec);
			if (ec)
			{
				canonical = std::filesystem::absolute(std::filesystem::path(path), ec);
				if (ec)
				{
					canonical = std::filesystem::path(path);
				}
			}
			return LowerAscii(canonical.lexically_normal().string());
		}

		// Maps a 002d load failure onto this layer's vocabulary.
		//
		// The two statuses worth singling out:
		//
		//   * `NoNavigation` stays a SUCCESS. `bExist == 0` is a valid file with
		//     nothing in it, and the nine shipped files that have it are the
		//     character-select and login maps. Calling that malformed would turn
		//     nine good maps into nine errors.
		//   * An `InvalidFile` whose stage is `FileID` becomes `UnsupportedWld`
		//     rather than `MalformedWld`, because `square_rd.wld`'s 0x0202 is a
		//     version legacy itself refuses (DxLandManSaveLoad.cpp:3165-3190). An
		//     operator chasing "corrupt file" for it would waste a day.
		MapResolveStatus TranslateNavigationStatus(Navigation::NavigationLoadStatus status,
		                                          const std::string& stage,
		                                          MapResolveStatus& out)
		{
			switch (status)
			{
			case Navigation::NavigationLoadStatus::Loaded:
				out = MapResolveStatus::Loaded;
				return out;
			case Navigation::NavigationLoadStatus::NoNavigation:
				out = MapResolveStatus::NoNavigation;
				return out;
			case Navigation::NavigationLoadStatus::DecryptionFailed:
				out = MapResolveStatus::UndecryptableWld;
				return out;
			case Navigation::NavigationLoadStatus::UnsupportedFormat:
				out = MapResolveStatus::UnsupportedWld;
				return out;
			case Navigation::NavigationLoadStatus::InvalidFile:
				out = stage == "FileID" ? MapResolveStatus::UnsupportedWld
				                        : MapResolveStatus::MalformedWld;
				return out;
			case Navigation::NavigationLoadStatus::Truncated:
			case Navigation::NavigationLoadStatus::Inconsistent:
			default:
				out = MapResolveStatus::MalformedWld;
				return out;
			}
		}
	}

	const char* ToString(MapResolveStatus status) noexcept
	{
		switch (status)
		{
		case MapResolveStatus::Loaded:           return "Loaded";
		case MapResolveStatus::NotRegistered:    return "NotRegistered";
		case MapResolveStatus::Ineligible:       return "Ineligible";
		case MapResolveStatus::MissingLev:       return "MissingLev";
		case MapResolveStatus::MalformedLev:     return "MalformedLev";
		case MapResolveStatus::MissingWld:       return "MissingWld";
		case MapResolveStatus::MalformedWld:     return "MalformedWld";
		case MapResolveStatus::UnsupportedWld:   return "UnsupportedWld";
		case MapResolveStatus::NoNavigation:     return "NoNavigation";
		case MapResolveStatus::UndecryptableWld: return "UndecryptableWld";
		}
		return "Unknown";
	}

	MapRegistry::MapRegistry(std::string assetRoot)
	    : m_assetRoot(std::move(assetRoot))
	{
	}

	bool MapRegistry::Load()
	{
		m_loaded    = false;
		m_loadError.clear();
		m_summary   = MapRegistrySummary{};
		m_entries.clear();
		m_index.clear();

		if (m_assetRoot.empty())
		{
			m_loadError = "no asset root was supplied";
			return false;
		}

		const std::filesystem::path root(m_assetRoot);

		const std::string dataDirectory =
		    FirstExistingDirectory(root, {"Data", "data"});
		if (dataDirectory.empty())
		{
			m_loadError = "asset root '" + m_assetRoot + "' has no Data directory";
			return false;
		}

		// `mapslist.mst` lives in the GLogic directory: `GLMapList::LoadMapsListFile`
		// builds its path as `GLOGIC::GetPath() + m_szFileName`
		// (GLMapList.cpp:112-114), and `GLOGIC_FILE` is `\Data\GLogic\`
		// (SUBPATH.cpp:30).
		const std::string glogicDirectory =
		    FirstExistingDirectory(root, {"Data/GLogic", "Data/Glogic", "Data/GLOGIC",
		                                  "data/GLogic", "data/Glogic", "data/glogic",
		                                  "GLogic", "glogic"});
		if (glogicDirectory.empty())
		{
			m_loadError = "asset root '" + m_assetRoot + "' has no Data/GLogic directory";
			return false;
		}

		const std::string levelDirectory =
		    FirstExistingDirectory(root, {"Data/GLogic/Level", "Data/Glogic/Level",
		                                  "Data/GLOGIC/LEVEL", "data/GLogic/Level",
		                                  "data/Glogic/Level", "data/glogic/level",
		                                  "GLogic/Level", "glogic/level"});
		if (levelDirectory.empty())
		{
			m_loadError = "asset root '" + m_assetRoot + "' has no Data/GLogic/Level directory";
			return false;
		}

		// `GLLandMan` opens the `.wld` under the map directory
		// (GLLandManSet.cpp:19-20); the same candidates the 002d tests use.
		const std::string mapDirectory =
		    FirstExistingDirectory(root, {"Data/Map", "data/Map", "Data/MAP", "data/map",
		                                  "Map", "map"});
		if (mapDirectory.empty())
		{
			m_loadError = "asset root '" + m_assetRoot + "' has no Data/Map directory";
			return false;
		}

		const std::string mapsListPath =
		    ResolveFileCaseInsensitively(glogicDirectory, "mapslist.mst");
		if (mapsListPath.empty())
		{
			m_loadError = "'" + glogicDirectory + "' has no mapslist.mst";
			return false;
		}

		std::vector<std::uint8_t> mapsListBytes;
		if (!ReadWholeFile(mapsListPath, mapsListBytes))
		{
			m_loadError = "mapslist.mst could not be read: " + mapsListPath;
			return false;
		}

		MapsListFile decoded;
		MapsListError decodeError;
		if (!DecodeMapsList(mapsListBytes.data(), mapsListBytes.size(), decoded,
		                    decodeError))
		{
			// Without `mapslist.mst` there is no map list at all, so this - unlike an
			// individual map failure - genuinely fails the load. The decoder's own
			// message is carried through verbatim: it names the stage, says why, and
			// gives an offset, which is what turns "the registry will not load" into
			// an actionable report.
			m_loadError = decodeError.Format(mapsListPath);
			return false;
		}

		m_summary.recordsRead = decoded.records.size();

		m_entries.reserve(decoded.records.size());
		m_index.reserve(decoded.records.size());

		// The shared-WLD cache. Populated during construction and never written
		// again, so the read path needs no lock - see MapRegistry.h.
		std::unordered_map<std::string, std::shared_ptr<const Navigation::NavigationMesh>> meshCache;

		for (const MapRecord& record : decoded.records)
		{
			MapEntry entry;
			entry.identity        = record.identity;
			entry.levelFileName   = record.levelFileName;
			entry.mapName         = record.mapName;
			entry.fieldServerId   = record.fieldServerId;

			// RAN's registration filter, applied in the order `LoadMapsListFile`
			// applies it (GLMapList.cpp:162-177): `bUsed`, the id range, the name
			// length. A record that fails any of them is present in the file and
			// skipped by legacy, and saying so is more useful than counting it as
			// registered.
			if (!record.RegistrationEligible())
			{
				entry.status = MapResolveStatus::Ineligible;
				entry.detail = "record rejected by RAN's registration filter";
			}
			else
			{
				++m_summary.registered;
				ResolveOne(record, levelDirectory, mapDirectory, meshCache, entry);
			}

			m_index[entry.identity.Packed()] = m_entries.size();
			m_entries.push_back(std::move(entry));
		}

		m_summary.distinctWlds = meshCache.size();

		Recount();
		m_loaded = true;
		return true;
	}

	const MapEntry* MapRegistry::Find(MapIdentity id) const
	{
		const auto it = m_index.find(id.Packed());
		if (it == m_index.end())
		{
			return nullptr;
		}
		return &m_entries[it->second];
	}

	std::shared_ptr<const Navigation::NavigationMesh> MapRegistry::NavigationMesh(MapIdentity id) const
	{
		const MapEntry* entry = Find(id);
		if (entry == nullptr)
		{
			return nullptr;
		}
		return entry->mesh;
	}

	std::string MapRegistry::WldPath(MapIdentity id) const
	{
		const MapEntry* entry = Find(id);
		if (entry == nullptr)
		{
			return std::string();
		}
		return entry->wldPath;
	}

	void MapRegistry::ResolveOne(const MapRecord& record, const std::string& levelDirectory,
	                             const std::string& mapDirectory,
	                             std::unordered_map<std::string,
	                                               std::shared_ptr<const Navigation::NavigationMesh>>& meshCache,
	                             MapEntry& entry)
	{
		// ---- the .lev -------------------------------------------------------
		const std::string levelPath =
		    ResolveFileCaseInsensitively(levelDirectory, record.levelFileName);
		if (levelPath.empty())
		{
			entry.status = MapResolveStatus::MissingLev;
			entry.detail = "level directory '" + levelDirectory + "' has no file named " +
			               record.levelFileName;
			return;
		}

		std::vector<std::uint8_t> levelBytes;
		if (!ReadWholeFile(levelPath, levelBytes))
		{
			entry.status = MapResolveStatus::MalformedLev;
			entry.detail = "the .lev could not be read: " + levelPath;
			return;
		}

		LevHead     head;
		LevHeadError headError;
		if (!DecodeLevHead(levelBytes.data(), levelBytes.size(), head, headError))
		{
			entry.status = MapResolveStatus::MalformedLev;
			entry.detail = headError.Format(record.levelFileName);
			return;
		}

		entry.wldFileName = head.wldFileName;

		// ---- the .wld -------------------------------------------------------
		const std::string wldPath = ResolveFileCaseInsensitively(mapDirectory, head.wldFileName);
		if (wldPath.empty())
		{
			entry.status = MapResolveStatus::MissingWld;
			entry.detail = "map directory '" + mapDirectory + "' has no file named " +
			               head.wldFileName;
			return;
		}

		entry.wldPath = wldPath;

		// ---- the mesh, shared ----------------------------------------------
		//
		// 002c measured 16 of the 56 distinct `.wld` files named by the 99
		// registered maps being named by more than one map id - `suhak.wld` serves
		// three, `w_3school_bilding_inner01.wld` serves eight. So this lookup is not
		// an optimisation, it is the correct semantics: one file, one mesh, many map
		// ids. Loading per map id would build 99 meshes where 56 are needed and give
		// each map id its own private copy of state that is immutable anyway.
		//
		// The key is the canonical, case-folded path of the file that was actually
		// opened, so two spellings of one file cannot produce two meshes.
		const std::string cacheKey = CanonicalCacheKey(wldPath);

		const auto cached = meshCache.find(cacheKey);
		if (cached != meshCache.end())
		{
			entry.status = MapResolveStatus::Loaded;
			entry.mesh   = cached->second;
			entry.detail = "shared navigation mesh";
			return;
		}

		// 002d's loader, unmodified. It owns its own decryption (including the
		// two-pass case `es_f41_killbillzone01.wld` needs) and its own `bExist == 0`
		// handling; nothing here re-decrypts a `.wld` or second-guesses its verdict.
		Navigation::NavigationLoadResult loaded =
		    Navigation::LoadWldNavigationMesh(wldPath);

		if (!loaded.Ok())
		{
			entry.status = MapResolveStatus::MalformedWld;
			TranslateNavigationStatus(loaded.status, loaded.error.stage, entry.status);
			entry.detail = loaded.error.Format(head.wldFileName);
			return;
		}

		// `Loaded` always carries a mesh - `WldNavigationLoader` returns
		// `NoNavigation` instead when `bExist == 0`, so this cannot be null without
		// the loader having a bug. Checked anyway: a null here would become a
		// "successful" entry that crashes every reader, and a two-line guard is
		// cheaper than that.
		if (loaded.mesh == nullptr)
		{
			entry.status = MapResolveStatus::MalformedWld;
			entry.detail = head.wldFileName + ": reported Loaded with no mesh";
			return;
		}

		// Stored as `const` from the moment it is created. There is no path in this
		// class that yields a mutable `NavigationMesh`.
		std::shared_ptr<const Navigation::NavigationMesh> mesh = loaded.mesh;
		meshCache.emplace(cacheKey, mesh);

		entry.status = MapResolveStatus::Loaded;
		entry.mesh   = mesh;
		entry.detail = "navigation mesh built";
	}

	void MapRegistry::Recount()
	{
		std::size_t resolved = 0;
		std::size_t meshes   = 0;

		for (const MapEntry& entry : m_entries)
		{
			switch (entry.status)
			{
			case MapResolveStatus::Loaded:
				++resolved;
				++meshes;
				break;
			case MapResolveStatus::NotRegistered:  ++m_summary.notRegistered;  break;
			case MapResolveStatus::Ineligible:     ++m_summary.ineligible;     break;
			case MapResolveStatus::MissingLev:     ++m_summary.missingLev;     break;
			case MapResolveStatus::MalformedLev:   ++m_summary.malformedLev;   break;
			case MapResolveStatus::MissingWld:     ++m_summary.missingWld;     break;
			case MapResolveStatus::MalformedWld:   ++m_summary.malformedWld;   break;
			case MapResolveStatus::UnsupportedWld: ++m_summary.unsupportedWld; break;
			case MapResolveStatus::NoNavigation:   ++m_summary.noNavigation;   break;
			case MapResolveStatus::UndecryptableWld: ++m_summary.undecryptableWld; break;
			}
		}

		m_summary.resolved        = resolved;
		m_summary.navigationMeshes = meshes;
	}

	std::vector<std::string> MapRegistry::Report() const
	{
		std::vector<std::string> lines;
		lines.reserve(m_entries.size());

		for (const MapEntry& entry : m_entries)
		{
			std::string line = ToString(entry.identity);
			line += "  " + entry.levelFileName;
			if (!entry.wldFileName.empty())
			{
				line += " -> " + entry.wldFileName;
			}
			line += std::string("  [") + ToString(entry.status) + "]";
			if (!entry.detail.empty())
			{
				line += " " + entry.detail;
			}
			if (!entry.mapName.empty())
			{
				line += "  " + entry.mapName;
			}
			lines.push_back(std::move(line));
		}
		return lines;
	}
}
