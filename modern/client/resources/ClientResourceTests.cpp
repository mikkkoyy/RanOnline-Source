// Modern client resource boundary tests: CLIENT-005 (resource boundary) and
// CLIENT-006 (filesystem asset provider).
//
// CLIENT-005 coverage:
// 1. ResourceId rejects invalid/empty identifiers.
// 2. Valid ResourceId compares correctly.
// 3. ResourceData preserves byte contents.
// 4. MemoryResourceProvider can register a resource.
// 5. Existing resource can be loaded.
// 6. Missing resource returns NotFound.
// 7. Duplicate registration is deterministic.
// 8. Repeated lookup is deterministic.
// 9. ResourceManager uses its injected provider.
// 10. ResourceManager does not require a renderer.
// 11. ResourceManager can operate headlessly.
// 12. Shutdown/lifecycle behavior is deterministic.
// 13. Invalid operations return errors rather than throwing.
// 14. No legacy headers are required.
//
// CLIENT-006 coverage (FileSystemResourceProvider):
// 1.  A valid root directory initializes.
// 2.  A missing or invalid root is rejected.
// 3.  An existing binary file is found.
// 4.  An existing binary file loads correctly.
// 5.  An empty file behaves deterministically.
// 6.  A missing file returns NotFound.
// 7.  A directory is not treated as a resource.
// 8.  Nested resource paths work.
// 9.  Forward-slash identifiers resolve.
// 10. "../" traversal is rejected.
// 11. Nested traversal is rejected.
// 12. Absolute-path attempts are rejected.
// 13. Drive-letter path attempts are rejected.
// 14. A file outside the root cannot be loaded.
// 15. HasResource() does not read file contents.
// 16. ResourceManager can use FileSystemResourceProvider.
// 17. ResourceManager caching still works.
// 18. ResourceManager remains independent of rendering.
// 19. No RAN installation is required.
// 20. No legacy headers are required.
//
// Every filesystem test builds its own root under the system temporary
// directory and removes it again, so nothing here reads, writes or depends on
// a RAN client installation.

#include "TestHarness.h"

#include "resources/FileSystemResourceProvider.h"
#include "resources/MemoryResourceProvider.h"
#include "resources/ResourceData.h"
#include "resources/ResourceId.h"
#include "resources/ResourceManager.h"
#include "resources/ResourceProvider.h"
#include "types/Result.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

// ---------------------------------------------------------------------------
// 1 & 2: ResourceId validation and comparison
// ---------------------------------------------------------------------------

MODERN_TEST(ClientResources_ResourceIdRejectsInvalidOrEmpty)
{
	auto res1 = ResourceId::Create("");
	CHECK(res1.IsError());
	CHECK_EQ(res1.GetError(), ErrorCode::InvalidArgument);

	auto res2 = ResourceId::Create("   \t\n");
	CHECK(res2.IsError());
	CHECK_EQ(res2.GetError(), ErrorCode::InvalidArgument);

	auto res3 = ResourceId::Create("textures\\ui\\login");
	CHECK(res3.IsError());
	CHECK_EQ(res3.GetError(), ErrorCode::InvalidArgument);

	auto res4 = ResourceId::Create("/textures/ui/login");
	CHECK(res4.IsError());
	CHECK_EQ(res4.GetError(), ErrorCode::InvalidArgument);

	auto res5 = ResourceId::Create("textures/ui/login/");
	CHECK(res5.IsError());
	CHECK_EQ(res5.GetError(), ErrorCode::InvalidArgument);

	ResourceId defaultId;
	CHECK(!defaultId.IsValid());
}

MODERN_TEST(ClientResources_ValidResourceIdComparesCorrectly)
{
	auto res1 = ResourceId::Create("textures/ui/login");
	auto res2 = ResourceId::Create("textures/ui/login");
	auto res3 = ResourceId::Create("textures/ui/char_select");

	CHECK(res1.IsOk());
	CHECK(res2.IsOk());
	CHECK(res3.IsOk());

	ResourceId id1 = res1.GetValue();
	ResourceId id2 = res2.GetValue();
	ResourceId id3 = res3.GetValue();

	CHECK(id1.IsValid());
	CHECK_EQ(id1, id2);
	CHECK(id1 != id3);
	CHECK(id3 < id1);

	CHECK_EQ(id1.GetName(), "textures/ui/login");
	CHECK_EQ(id1.GetView(), "textures/ui/login");
}

// ---------------------------------------------------------------------------
// 3: ResourceData preserves byte contents
// ---------------------------------------------------------------------------

MODERN_TEST(ClientResources_ResourceDataPreservesByteContents)
{
	const std::vector<uint8_t> raw = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03 };
	ResourceData data(raw);

	CHECK(!data.IsEmpty());
	CHECK_EQ(data.GetSize(), raw.size());
	CHECK(data.GetBytes() == raw);
	CHECK_EQ(data.GetData()[0], 0xDE);
	CHECK_EQ(data.GetData()[3], 0xEF);

	std::string text = "RAN_ONLINE_TEST_PAYLOAD";
	ResourceData textData(text);
	CHECK_EQ(textData.GetSize(), text.size());
	CHECK_EQ(textData.AsStringView(), text);

	ResourceData textData2(text);
	CHECK_EQ(textData, textData2);
	CHECK(data != textData);

	ResourceData emptyData;
	CHECK(emptyData.IsEmpty());
	CHECK_EQ(emptyData.GetSize(), 0u);
}

// ---------------------------------------------------------------------------
// 4, 5, 6, 7: MemoryResourceProvider functionality
// ---------------------------------------------------------------------------

MODERN_TEST(ClientResources_MemoryProviderRegisterAndLoadExisting)
{
	MemoryResourceProvider provider;
	CHECK_EQ(provider.GetCount(), 0u);

	auto idRes = ResourceId::Create("test/mock_data");
	CHECK(idRes.IsOk());
	ResourceId id = idRes.GetValue();

	CHECK(!provider.HasResource(id));

	std::string payload = "binary_mock_contents";
	ResourceData data(payload);

	Status regStatus = provider.RegisterResource(id, data);
	CHECK(regStatus.IsOk());
	CHECK_EQ(provider.GetCount(), 1u);
	CHECK(provider.HasResource(id));

	auto loadRes = provider.Load(id);
	CHECK(loadRes.IsOk());
	CHECK_EQ(loadRes.GetValue().AsStringView(), payload);
}

MODERN_TEST(ClientResources_MemoryProviderMissingReturnsNotFound)
{
	MemoryResourceProvider provider;

	auto idRes = ResourceId::Create("missing/resource");
	CHECK(idRes.IsOk());
	ResourceId id = idRes.GetValue();

	CHECK(!provider.HasResource(id));
	auto loadRes = provider.Load(id);
	CHECK(loadRes.IsError());
	CHECK_EQ(loadRes.GetError(), ErrorCode::NotFound);

	Status unregStatus = provider.UnregisterResource(id);
	CHECK(unregStatus.IsError());
	CHECK_EQ(unregStatus.GetCode(), ErrorCode::NotFound);
}

MODERN_TEST(ClientResources_MemoryProviderDuplicateRegistrationIsDeterministic)
{
	MemoryResourceProvider provider;
	auto id = ResourceId::Create("config/settings").GetValue();

	ResourceData v1("version_1");
	ResourceData v2("version_2");

	CHECK(provider.RegisterResource(id, v1).IsOk());
	CHECK_EQ(provider.Load(id).GetValue().AsStringView(), "version_1");

	CHECK(provider.RegisterResource(id, v2).IsOk());
	CHECK_EQ(provider.GetCount(), 1u);
	CHECK_EQ(provider.Load(id).GetValue().AsStringView(), "version_2");

	CHECK(provider.UnregisterResource(id).IsOk());
	CHECK_EQ(provider.GetCount(), 0u);
	CHECK(!provider.HasResource(id));
}

// ---------------------------------------------------------------------------
// 8, 9, 10, 11: ResourceManager Coordination, Cache, and Headless Execution
// ---------------------------------------------------------------------------

MODERN_TEST(ClientResources_ResourceManagerUsesInjectedProviderAndCaches)
{
	MemoryResourceProvider provider;
	auto idA = ResourceId::Create("textures/terrain").GetValue();
	auto idB = ResourceId::Create("textures/water").GetValue();

	provider.RegisterResource(idA, ResourceData("TERRAIN_MAP"));
	provider.RegisterResource(idB, ResourceData("WATER_MAP"));

	ResourceManager manager;
	CHECK_EQ(manager.GetState(), ResourceManagerState::Uninitialized);

	manager.SetProvider(&provider);
	CHECK_EQ(manager.GetProvider(), &provider);

	Status initStatus = manager.Initialize();
	CHECK(initStatus.IsOk());
	CHECK_EQ(manager.GetState(), ResourceManagerState::Ready);

	CHECK_EQ(manager.GetCachedCount(), 0u);
	CHECK(!manager.IsCached(idA));
	CHECK(!manager.IsCached(idB));

	auto loadA = manager.Load(idA);
	CHECK(loadA.IsOk());
	CHECK_EQ(loadA.GetValue().AsStringView(), "TERRAIN_MAP");
	CHECK_EQ(manager.GetCachedCount(), 1u);
	CHECK(manager.IsCached(idA));

	auto loadA2 = manager.Load(idA);
	CHECK(loadA2.IsOk());
	CHECK_EQ(loadA2.GetValue().AsStringView(), "TERRAIN_MAP");
	CHECK_EQ(manager.GetCachedCount(), 1u);

	auto loadB = manager.Load(idB);
	CHECK(loadB.IsOk());
	CHECK_EQ(loadB.GetValue().AsStringView(), "WATER_MAP");
	CHECK_EQ(manager.GetCachedCount(), 2u);
	CHECK(manager.IsCached(idB));

	CHECK(manager.ClearCache().IsOk());
	CHECK_EQ(manager.GetCachedCount(), 0u);
	CHECK(!manager.IsCached(idA));
	CHECK(!manager.IsCached(idB));
}

MODERN_TEST(ClientResources_ResourceManagerHeadlessWithoutRenderer)
{
	MemoryResourceProvider provider;
	auto id = ResourceId::Create("scripts/dialogue").GetValue();
	provider.RegisterResource(id, ResourceData("DIALOGUE_TREE_DATA"));

	ResourceManager manager;
	manager.SetProvider(&provider);
	CHECK(manager.Initialize().IsOk());

	auto result = manager.Load(id);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().AsStringView(), "DIALOGUE_TREE_DATA");
	CHECK(manager.Shutdown().IsOk());
}

// ---------------------------------------------------------------------------
// 12 & 13: Lifecycle and Error Handling without Throwing
// ---------------------------------------------------------------------------

MODERN_TEST(ClientResources_ResourceManagerLifecycleTransitions)
{
	ResourceManager manager;
	CHECK_EQ(manager.GetState(), ResourceManagerState::Uninitialized);
	CHECK_EQ(std::string(ToString(manager.GetState())), "Uninitialized");

	// Initializing without provider returns InvalidArgument
	Status initNoProvider = manager.Initialize();
	CHECK(initNoProvider.IsError());
	CHECK_EQ(initNoProvider.GetCode(), ErrorCode::InvalidArgument);

	MemoryResourceProvider provider;
	manager.SetProvider(&provider);

	CHECK(manager.Initialize().IsOk());
	CHECK_EQ(manager.GetState(), ResourceManagerState::Ready);
	CHECK_EQ(std::string(ToString(manager.GetState())), "Ready");

	// Re-initializing returns InvalidState
	Status reinit = manager.Initialize();
	CHECK(reinit.IsError());
	CHECK_EQ(reinit.GetCode(), ErrorCode::InvalidState);

	// Shutdown
	CHECK(manager.Shutdown().IsOk());
	CHECK_EQ(manager.GetState(), ResourceManagerState::ShutdownState);
	CHECK_EQ(std::string(ToString(manager.GetState())), "Shutdown");

	// Operations after shutdown return NotAllowed
	Status postInit = manager.Initialize();
	CHECK_EQ(postInit.GetCode(), ErrorCode::NotAllowed);

	Status postShutdown = manager.Shutdown();
	CHECK_EQ(postShutdown.GetCode(), ErrorCode::NotAllowed);

	auto id = ResourceId::Create("test/res").GetValue();
	auto postLoad = manager.Load(id);
	CHECK_EQ(postLoad.GetError(), ErrorCode::InvalidState);
}

MODERN_TEST(ClientResources_InvalidOperationsReturnErrorsNotThrow)
{
	MemoryResourceProvider provider;
	ResourceManager manager;
	manager.SetProvider(&provider);

	// Load before Initialize returns InvalidState
	ResourceId defaultId;
	auto loadBeforeInit = manager.Load(defaultId);
	CHECK_EQ(loadBeforeInit.GetError(), ErrorCode::InvalidState);

	CHECK(manager.Initialize().IsOk());

	// Load with invalid default id returns InvalidArgument
	auto loadInvalid = manager.Load(defaultId);
	CHECK_EQ(loadInvalid.GetError(), ErrorCode::InvalidArgument);

	// Provider with invalid id
	CHECK_EQ(provider.RegisterResource(defaultId, ResourceData()).GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(provider.UnregisterResource(defaultId).GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(provider.Load(defaultId).GetError(), ErrorCode::InvalidArgument);
	CHECK(!provider.HasResource(defaultId));
}

// ---------------------------------------------------------------------------
// CLIENT-006 fixtures: a private root per test, under the system temp directory
// ---------------------------------------------------------------------------

namespace
{
	unsigned NextSequence() noexcept
	{
		static unsigned sequence = 0;
		return ++sequence;
	}

	// A private asset root for one test, removed on destruction.
	//
	// The tests own their root so that they run on any machine and so that a
	// test which returns early still cleans up. Base() is the container; Path()
	// is the directory actually configured as the provider root, one level
	// below, which leaves somewhere for a traversal test to point at a file
	// that genuinely exists outside the root.
	class TempRoot
	{
	public:
		TempRoot()
		{
			std::error_code ec;
			const std::filesystem::path systemTemp = std::filesystem::temp_directory_path(ec);
			if (ec)
			{
				return;
			}

			const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
			m_base = systemTemp / ("modern_client006_" + std::to_string(stamp) + "_" + std::to_string(NextSequence()));

			std::filesystem::remove_all(m_base, ec);
			ec.clear();
			std::filesystem::create_directories(m_base, ec);
			if (ec)
			{
				m_base.clear();
				return;
			}

			m_root = m_base / "root";
			ec.clear();
			std::filesystem::create_directories(m_root, ec);
			if (ec)
			{
				m_root.clear();
			}
		}

		~TempRoot()
		{
			std::error_code ec;
			std::filesystem::remove_all(m_base, ec);
		}

		TempRoot(const TempRoot&) = delete;
		TempRoot& operator=(const TempRoot&) = delete;

		const std::filesystem::path& Base() const noexcept { return m_base; }
		const std::filesystem::path& Path() const noexcept { return m_root; }
		bool IsUsable() const noexcept { return !m_base.empty() && !m_root.empty(); }

		bool Write(const std::filesystem::path& relative, const std::vector<uint8_t>& bytes) const
		{
			return WriteInto(m_root, relative, bytes);
		}

		bool WriteText(const std::filesystem::path& relative, std::string_view text) const
		{
			return Write(relative, std::vector<uint8_t>(text.begin(), text.end()));
		}

		// Writes above the configured root, so a traversal test can aim at a
		// file that really is there.
		bool WriteOutside(const std::filesystem::path& relative, const std::vector<uint8_t>& bytes) const
		{
			return WriteInto(m_base, relative, bytes);
		}

		bool MakeDirectory(const std::filesystem::path& relative) const
		{
			std::error_code ec;
			std::filesystem::create_directories(m_root / relative, ec);
			return !ec;
		}

	private:
		static bool WriteInto(
			const std::filesystem::path& base,
			const std::filesystem::path& relative,
			const std::vector<uint8_t>& bytes)
		{
			const std::filesystem::path target = base / relative;

			std::error_code ec;
			std::filesystem::create_directories(target.parent_path(), ec);
			if (ec)
			{
				return false;
			}

			std::ofstream out(target, std::ios::binary | std::ios::trunc);
			if (!out)
			{
				return false;
			}

			if (!bytes.empty())
			{
				out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
			}

			return static_cast<bool>(out);
		}

		std::filesystem::path m_base;
		std::filesystem::path m_root;
	};

	// Deterministic, byte-value-covering content: a mix-up between two files
	// shows up as a mismatch rather than as an accidental pass.
	std::vector<uint8_t> MakePattern(size_t size)
	{
		std::vector<uint8_t> bytes(size);
		for (size_t i = 0; i < size; ++i)
		{
			bytes[i] = static_cast<uint8_t>((i * 31u + 7u) % 251u);
		}
		return bytes;
	}

	// Builds a ResourceId, asserting that the identifier layer accepts it. Some
	// CLIENT-006 cases depend on names ResourceId::Create() allows ("..",
	// "C:/...") so that the provider's own refusal is what is under test.
	ResourceId MakeId(const char* name)
	{
		const Result<ResourceId> id = ResourceId::Create(name);
		CHECK(id.IsOk());
		return id.GetValueOr(ResourceId());
	}

	// Filesystem queries for assertions. The error-code overloads keep every
	// filesystem call in this file non-throwing, including the checks.
	bool Exists(const std::filesystem::path& path)
	{
		std::error_code ec;
		return std::filesystem::exists(path, ec);
	}

	bool IsRegularFile(const std::filesystem::path& path)
	{
		std::error_code ec;
		return std::filesystem::is_regular_file(path, ec);
	}

	bool IsDirectory(const std::filesystem::path& path)
	{
		std::error_code ec;
		return std::filesystem::is_directory(path, ec);
	}
}

// ---------------------------------------------------------------------------
// CLIENT-006 1-4: root configuration, discovery and binary loading
// ---------------------------------------------------------------------------

MODERN_TEST(ClientFilesystem_ValidRootInitializes)
{
	TempRoot temp;
	CHECK(temp.IsUsable());

	FileSystemResourceProvider provider(temp.Path());

	// Constructing a provider touches nothing: no I/O happens until Initialize.
	CHECK(!provider.IsInitialized());
	CHECK_EQ(provider.GetConfiguredRoot().string(), temp.Path().string());
	CHECK(provider.GetResolvedRoot().empty());

	const Status status = provider.Initialize();
	CHECK(status.IsOk());
	CHECK(provider.IsInitialized());
	CHECK(provider.GetResolvedRoot().is_absolute());
	CHECK(!provider.GetResolvedRoot().empty());

	// The configured root is remembered as given; the resolved root is what
	// every later comparison uses.
	CHECK_EQ(provider.GetConfiguredRoot().string(), temp.Path().string());

	// Initializing twice is a state error rather than a second root.
	CHECK_EQ(provider.Initialize().GetCode(), ErrorCode::InvalidState);
	CHECK(provider.IsInitialized());
}

MODERN_TEST(ClientFilesystem_MissingOrInvalidRootIsRejected)
{
	TempRoot temp;
	CHECK(temp.IsUsable());

	// An empty root can never address anything.
	FileSystemResourceProvider emptyRoot{ std::filesystem::path() };
	CHECK_EQ(emptyRoot.Initialize().GetCode(), ErrorCode::InvalidArgument);
	CHECK(!emptyRoot.IsInitialized());

	// A root that does not exist is refused, and is not created on the way.
	const std::filesystem::path missing = temp.Path() / "does_not_exist";
	FileSystemResourceProvider missingRoot(missing);
	CHECK_EQ(missingRoot.Initialize().GetCode(), ErrorCode::NotFound);
	CHECK(!missingRoot.IsInitialized());
	CHECK(!Exists(missing));

	// A regular file is not a directory, so it is not a root.
	CHECK(temp.WriteText("not_a_directory.bin", "FILE"));
	FileSystemResourceProvider fileRoot(temp.Path() / "not_a_directory.bin");
	CHECK_EQ(fileRoot.Initialize().GetCode(), ErrorCode::InvalidArgument);
	CHECK(!fileRoot.IsInitialized());

	// None of the refusals left the provider half-usable.
	CHECK(!emptyRoot.IsInitialized());
	CHECK(!missingRoot.IsInitialized());
	CHECK(!fileRoot.IsInitialized());
}

MODERN_TEST(ClientFilesystem_ExistingBinaryFileIsFound)
{
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.WriteText("ui/login.bin", "LOGIN_BINARY"));

	FileSystemResourceProvider provider(temp.Path());

	// An uninitialised provider answers "no" rather than throwing or guessing a
	// directory.
	const ResourceId id = MakeId("ui/login.bin");
	CHECK(!provider.HasResource(id));

	CHECK(provider.Initialize().IsOk());
	CHECK(provider.HasResource(id));

	// The file really is where the identifier said it was, so the answer above
	// came from the file and not from a resolution accident.
	CHECK(IsRegularFile(temp.Path() / "ui" / "login.bin"));
}

MODERN_TEST(ClientFilesystem_ExistingBinaryFileLoadsCorrectly)
{
	TempRoot temp;
	CHECK(temp.IsUsable());

	// Every byte value, including 0x0A and 0x0D: a text-mode read or a
	// truncated buffer shows up immediately.
	std::vector<uint8_t> payload(256);
	for (size_t i = 0; i < payload.size(); ++i)
	{
		payload[i] = static_cast<uint8_t>(i);
	}
	CHECK(temp.Write("ui/atlas.bin", payload));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	const auto loaded = provider.Load(MakeId("ui/atlas.bin"));
	CHECK(loaded.IsOk());
	CHECK_EQ(loaded.GetValue().GetSize(), payload.size());
	CHECK(loaded.GetValue().GetBytes() == payload);
	CHECK_EQ(loaded.GetValue().GetData()[0x00], 0x00);
	CHECK_EQ(loaded.GetValue().GetData()[0x1F], 0x1F);
	CHECK_EQ(loaded.GetValue().GetData()[0x0A], 0x0A);
	CHECK_EQ(loaded.GetValue().GetData()[0xFF], 0xFF);

	// A second load returns the same bytes: the provider re-reads, and reading
	// is deterministic.
	const auto again = provider.Load(MakeId("ui/atlas.bin"));
	CHECK(again.IsOk());
	CHECK(again.GetValue().GetBytes() == payload);
}

// ---------------------------------------------------------------------------
// CLIENT-006 5-9: empty, missing, directory, nested and forward-slash cases
// ---------------------------------------------------------------------------

MODERN_TEST(ClientFilesystem_EmptyFileIsDeterministic)
{
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.Write("empty.bin", std::vector<uint8_t>()));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	const ResourceId id = MakeId("empty.bin");

	// An empty file is a resource that exists and contains nothing.
	CHECK(provider.HasResource(id));

	const auto first = provider.Load(id);
	CHECK(first.IsOk());
	CHECK(first.GetValue().IsEmpty());
	CHECK_EQ(first.GetValue().GetSize(), 0u);
	CHECK(first.GetValue().GetBytes().empty());

	// Zero bytes is a value, not an error: asking twice gives the same answer.
	const auto second = provider.Load(id);
	CHECK(second.IsOk());
	CHECK_EQ(second.GetValue().GetSize(), 0u);
	CHECK_EQ(first.GetValue(), second.GetValue());
}

MODERN_TEST(ClientFilesystem_MissingFileReturnsNotFound)
{
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.WriteText("present.bin", "HERE"));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	// Missing next to a file that exists.
	const ResourceId absent = MakeId("ui/absent.bin");
	CHECK(!provider.HasResource(absent));
	CHECK_EQ(provider.Load(absent).GetError(), ErrorCode::NotFound);

	// A file whose parent directory does not exist is also simply not found,
	// rather than an error about the directory.
	const ResourceId noDirectory = MakeId("nothing/here.bin");
	CHECK(!provider.HasResource(noDirectory));
	CHECK_EQ(provider.Load(noDirectory).GetError(), ErrorCode::NotFound);

	// The sibling that does exist is unaffected.
	CHECK(provider.HasResource(MakeId("present.bin")));
}

MODERN_TEST(ClientFilesystem_DirectoryIsNotAResource)
{
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.MakeDirectory("ui"));
	CHECK(temp.MakeDirectory("ui/nested"));
	CHECK(temp.WriteText("ui/login.bin", "LOGIN"));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	// "ui" exists, but as a directory: not something a provider may hand out.
	const ResourceId directory = MakeId("ui");
	CHECK(!provider.HasResource(directory));
	CHECK_EQ(provider.Load(directory).GetError(), ErrorCode::NotFound);

	const ResourceId nested = MakeId("ui/nested");
	CHECK(!provider.HasResource(nested));
	CHECK_EQ(provider.Load(nested).GetError(), ErrorCode::NotFound);

	// The file inside the refused directory is still a resource, so the rule is
	// about file type and not about depth.
	CHECK(provider.HasResource(MakeId("ui/login.bin")));
	CHECK(IsDirectory(temp.Path() / "ui" / "nested"));
}

MODERN_TEST(ClientFilesystem_NestedResourcePathsWork)
{
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.WriteText("top.bin", "TOP"));
	CHECK(temp.WriteText("nested/data.bin", "NESTED"));
	CHECK(temp.WriteText("nested/deeper/still/data.bin", "DEEP"));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	const auto top = provider.Load(MakeId("top.bin"));
	CHECK(top.IsOk());
	CHECK_EQ(top.GetValue().AsStringView(), "TOP");

	const auto nested = provider.Load(MakeId("nested/data.bin"));
	CHECK(nested.IsOk());
	CHECK_EQ(nested.GetValue().AsStringView(), "NESTED");

	const ResourceId deepId = MakeId("nested/deeper/still/data.bin");
	CHECK(provider.HasResource(deepId));
	const auto deep = provider.Load(deepId);
	CHECK(deep.IsOk());
	CHECK_EQ(deep.GetValue().AsStringView(), "DEEP");

	// Two files with the same name in different directories stay distinct.
	CHECK(provider.HasResource(MakeId("nested/data.bin")));
	CHECK_EQ(provider.Load(MakeId("data.bin")).GetError(), ErrorCode::NotFound);
}

MODERN_TEST(ClientFilesystem_ForwardSlashIdentifierResolves)
{
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.WriteText("a/b/c.bin", "SLASHED"));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	const ResourceId id = MakeId("a/b/c.bin");

	// The logical name is preserved exactly as given: the provider translates
	// it for the platform, it does not rewrite the identifier.
	CHECK_EQ(id.GetName(), "a/b/c.bin");
	CHECK(provider.HasResource(id));

	const auto loaded = provider.Load(id);
	CHECK(loaded.IsOk());
	CHECK_EQ(loaded.GetValue().AsStringView(), "SLASHED");

	// The same namespace written with the platform separator is not accepted
	// anywhere: forward slashes are the canonical form.
	CHECK(ResourceId::Create("a\\b\\c.bin").IsError());
	CHECK(!provider.HasResource(ResourceId()));
}

// ---------------------------------------------------------------------------
// CLIENT-006 10-14: the resource id / path safety boundary
//
// These five cases are the security-relevant ones. Each builds a file that
// genuinely exists where the escape would land, so passing means the provider
// refused a reachable file rather than failing to find one.
// ---------------------------------------------------------------------------

MODERN_TEST(ClientFilesystem_ParentTraversalIsRejected)
{
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.WriteText("inside.bin", "INSIDE"));

	// A real file one directory above the root.
	CHECK(temp.WriteOutside("secret.bin", std::vector<uint8_t>{ 'S', 'E', 'C', 'R', 'E', 'T' }));
	CHECK(IsRegularFile(temp.Base() / "secret.bin"));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	// ResourceId::Create() accepts these names: ".." is only dangerous once a
	// provider turns it into a path, so the refusal has to come from here.
	const ResourceId parent = MakeId("../secret.bin");
	CHECK(!provider.HasResource(parent));
	CHECK_EQ(provider.Load(parent).GetError(), ErrorCode::InvalidArgument);

	// A bare ".." and a bare "." are refused for the same reason rather than
	// being normalised away.
	CHECK(!provider.HasResource(MakeId("..")));
	CHECK_EQ(provider.Load(MakeId("..")).GetError(), ErrorCode::InvalidArgument);
	CHECK(!provider.HasResource(MakeId(".")));
	CHECK_EQ(provider.Load(MakeId(".")).GetError(), ErrorCode::InvalidArgument);

	// The legitimate resource in the same root still loads.
	const auto inside = provider.Load(MakeId("inside.bin"));
	CHECK(inside.IsOk());
	CHECK_EQ(inside.GetValue().AsStringView(), "INSIDE");
}

MODERN_TEST(ClientFilesystem_NestedTraversalIsRejected)
{
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.MakeDirectory("ui/data"));
	CHECK(temp.WriteText("ui/data/real.bin", "REAL"));
	CHECK(temp.WriteOutside("outside.bin", std::vector<uint8_t>{ 'O', 'U', 'T' }));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	const char* const rejected[] = {
		"ui/data/../../../outside.bin",
		"ui/../outside.bin",
		"nested/../../outside.bin",
		"ui/data/../..",
		"../../outside.bin",
	};

	for (const char* name : rejected)
	{
		const ResourceId id = MakeId(name);
		CHECK(!provider.HasResource(id));
		CHECK_EQ(provider.Load(id).GetError(), ErrorCode::InvalidArgument);
	}

	// A legitimate sibling still loads, so the rule above is about ".." rather
	// than about path depth or the presence of a parent component.
	const auto real = provider.Load(MakeId("ui/data/real.bin"));
	CHECK(real.IsOk());
	CHECK_EQ(real.GetValue().AsStringView(), "REAL");
}

MODERN_TEST(ClientFilesystem_AbsolutePathAttemptsAreRejected)
{
	TempRoot temp;
	CHECK(temp.IsUsable());

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	// POSIX-style absolute names never even become a ResourceId: a leading or
	// trailing separator is refused by the identifier layer.
	CHECK(ResourceId::Create("/etc/passwd").IsError());
	CHECK(ResourceId::Create("//server/share/file").IsError());
	CHECK(ResourceId::Create("\\\\server\\share\\file").IsError());
	CHECK(ResourceId::Create("/").IsError());
	CHECK(ResourceId::Create("file.bin/").IsError());

	// A default-constructed id is invalid, and the provider answers "no" rather
	// than resolving anything.
	CHECK(!provider.HasResource(ResourceId()));
	CHECK_EQ(provider.Load(ResourceId()).GetError(), ErrorCode::InvalidArgument);

	// The provider re-checks the leading-separator rule itself, so an absolute
	// name could not reach the filesystem even if one arrived by another route.
	// ResourceId::Create() cannot produce such a name, which is why there is no
	// identifier to pass here: the two layers are checked separately, and this
	// case pins the outer one.
	CHECK(provider.IsInitialized());
}

MODERN_TEST(ClientFilesystem_DriveLetterPathsAreRejected)
{
	TempRoot temp;
	CHECK(temp.IsUsable());

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	// ResourceId::Create() only enforces the logical form, so a drive-letter
	// name is accepted by the identifier layer. Refusing it is the provider's
	// job, and this is the case that proves the provider does it.
	const Result<ResourceId> driveId = ResourceId::Create("C:/Windows/win.ini");
	CHECK(driveId.IsOk());
	CHECK(!provider.HasResource(driveId.GetValue()));
	CHECK_EQ(provider.Load(driveId.GetValue()).GetError(), ErrorCode::InvalidArgument);

	// A bare drive-relative name is refused too: "C:" addresses the current
	// directory on a drive, which is outside a root-qualified namespace.
	CHECK_EQ(provider.Load(MakeId("C:")).GetError(), ErrorCode::InvalidArgument);

	// A colon is refused anywhere, not only as a root name: an NTFS alternate
	// data stream ("login.bin:payload") is a different file.
	const ResourceId stream = MakeId("ui/login.bin:payload");
	CHECK(!provider.HasResource(stream));
	CHECK_EQ(provider.Load(stream).GetError(), ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientFilesystem_FileOutsideRootCannotBeLoaded)
{
	TempRoot temp;
	CHECK(temp.IsUsable());

	// Two decoys above the root, one of them in a sibling tree, so that an
	// implementation which simply concatenated strings would find them.
	CHECK(temp.WriteOutside("outside.bin", std::vector<uint8_t>{ 'O', 'U', 'T', 'S', 'I', 'D', 'E' }));
	CHECK(temp.WriteOutside("sibling/victim.bin", std::vector<uint8_t>{ 'V', 'I', 'C', 'T', 'I', 'M' }));
	CHECK(IsRegularFile(temp.Base() / "outside.bin"));
	CHECK(IsRegularFile(temp.Base() / "sibling" / "victim.bin"));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	const auto loaded = provider.Load(MakeId("../outside.bin"));
	CHECK(loaded.IsError());
	CHECK_EQ(loaded.GetError(), ErrorCode::InvalidArgument);

	const ResourceId sibling = MakeId("../sibling/victim.bin");
	CHECK(!provider.HasResource(sibling));
	CHECK_EQ(provider.Load(sibling).GetError(), ErrorCode::InvalidArgument);

	// Nothing above the root is reachable, and the file above it stays
	// invisible under its plain name too: without traversal it resolves inside
	// the root, where it does not exist.
	CHECK(!provider.HasResource(MakeId("outside.bin")));
	CHECK_EQ(provider.Load(MakeId("outside.bin")).GetError(), ErrorCode::NotFound);
	CHECK(!provider.HasResource(MakeId("sibling/victim.bin")));

	// And the decoys are still on disk, untouched by any of the attempts.
	CHECK(IsRegularFile(temp.Base() / "outside.bin"));
}

// ---------------------------------------------------------------------------
// CLIENT-006 15-17: metadata-only queries and ResourceManager integration
// ---------------------------------------------------------------------------

MODERN_TEST(ClientFilesystem_HasResourceDoesNotReadContents)
{
	TempRoot temp;
	CHECK(temp.IsUsable());

	// A file large enough that reading it would be a real cost, and two files
	// whose contents cannot influence a metadata query.
	const size_t largeSize = 1024u * 1024u;
	CHECK(temp.Write("large.bin", MakePattern(largeSize)));
	CHECK(temp.Write("garbage.bin", MakePattern(64)));
	CHECK(temp.Write("empty.bin", std::vector<uint8_t>()));
	CHECK(temp.MakeDirectory("folder"));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	// Size, content and emptiness change nothing: the answer follows from
	// metadata alone. An implementation that loaded the file would still pass
	// these checks, which is why the guarantee is also a statement about the
	// code path - HasResource() opens no stream - and why the large-file case
	// is here rather than a small one.
	CHECK(provider.HasResource(MakeId("large.bin")));
	CHECK(provider.HasResource(MakeId("garbage.bin")));
	CHECK(provider.HasResource(MakeId("empty.bin")));

	// The negative answers come from metadata too.
	CHECK(!provider.HasResource(MakeId("folder")));
	CHECK(!provider.HasResource(MakeId("never_written.bin")));
	CHECK(!provider.HasResource(MakeId("../outside.bin")));
	CHECK(!provider.HasResource(MakeId("large.bin/..")));

	// And the large file is still readable in full when it is asked for, so the
	// checks above were not passing because the file was inaccessible.
	const auto loaded = provider.Load(MakeId("large.bin"));
	CHECK(loaded.IsOk());
	CHECK_EQ(loaded.GetValue().GetSize(), largeSize);
	CHECK(loaded.GetValue().GetBytes() == MakePattern(largeSize));
}

MODERN_TEST(ClientFilesystem_ResourceManagerUsesFileSystemProvider)
{
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.WriteText("ui/login.bin", "LOGIN_FROM_DISK"));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	ResourceManager manager;
	manager.SetProvider(&provider);
	CHECK_EQ(manager.GetProvider(), &provider);
	CHECK(manager.Initialize().IsOk());

	const ResourceId id = MakeId("ui/login.bin");
	const auto loaded = manager.Load(id);
	CHECK(loaded.IsOk());
	CHECK_EQ(loaded.GetValue().AsStringView(), "LOGIN_FROM_DISK");
	CHECK(manager.IsCached(id));
	CHECK_EQ(manager.GetCachedCount(), 1u);

	// The provider's errors reach the caller unchanged: the manager does not
	// invent a new code above the provider.
	CHECK_EQ(manager.Load(MakeId("ui/absent.bin")).GetError(), ErrorCode::NotFound);
	CHECK_EQ(manager.Load(MakeId("../escape.bin")).GetError(), ErrorCode::InvalidArgument);

	CHECK(manager.Shutdown().IsOk());
}

MODERN_TEST(ClientFilesystem_ResourceManagerCacheStillWorks)
{
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.WriteText("config/settings.bin", "VERSION_1"));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	ResourceManager manager;
	manager.SetProvider(&provider);
	CHECK(manager.Initialize().IsOk());

	const ResourceId id = MakeId("config/settings.bin");

	const auto first = manager.Load(id);
	CHECK(first.IsOk());
	CHECK_EQ(first.GetValue().AsStringView(), "VERSION_1");
	CHECK(manager.IsCached(id));

	// The file changes on disk. The cached copy must not: the manager is the
	// only cache, and it consults the provider only on a miss.
	CHECK(temp.WriteText("config/settings.bin", "VERSION_2"));

	const auto cached = manager.Load(id);
	CHECK(cached.IsOk());
	CHECK_EQ(cached.GetValue().AsStringView(), "VERSION_1");
	CHECK_EQ(manager.GetCachedCount(), 1u);

	// The provider holds no cache of its own: asked directly, it re-reads.
	const auto reread = provider.Load(id);
	CHECK(reread.IsOk());
	CHECK_EQ(reread.GetValue().AsStringView(), "VERSION_2");

	// After a cache clear the manager goes back to the provider for the new
	// bytes, which is the behaviour the cache is supposed to have.
	CHECK(manager.ClearCache().IsOk());
	CHECK(!manager.IsCached(id));
	CHECK_EQ(manager.GetCachedCount(), 0u);

	const auto reloaded = manager.Load(id);
	CHECK(reloaded.IsOk());
	CHECK_EQ(reloaded.GetValue().AsStringView(), "VERSION_2");

	CHECK(manager.Shutdown().IsOk());
}

// ---------------------------------------------------------------------------
// CLIENT-006 18-21: independence, portability and lifecycle determinism
// ---------------------------------------------------------------------------

MODERN_TEST(ClientFilesystem_ResourceManagerRemainsIndependentOfRendering)
{
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.WriteText("world/heightmap.bin", "HEIGHTMAP_BYTES"));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	// No renderer, no device, no window, no decoded texture: bytes in, bytes
	// out. This target links ModernClientResources and Modern only, so a
	// rendering dependency would not compile or link in the first place (see
	// modern/client/resources/CMakeLists.txt).
	ResourceManager manager;
	manager.SetProvider(&provider);
	CHECK(manager.Initialize().IsOk());

	const auto loaded = manager.Load(MakeId("world/heightmap.bin"));
	CHECK(loaded.IsOk());
	CHECK_EQ(loaded.GetValue().AsStringView(), "HEIGHTMAP_BYTES");

	// The bytes arrive undecoded: nothing above the provider has interpreted
	// them, which is what keeps decoding out of this layer.
	CHECK_EQ(loaded.GetValue().GetSize(), 15u);

	CHECK(manager.Shutdown().IsOk());
}

MODERN_TEST(ClientFilesystem_NoRanInstallationRequired)
{
	TempRoot temp;
	CHECK(temp.IsUsable());

	// Everything this test touches lives under the system temporary directory.
	std::error_code ec;
	const std::filesystem::path systemTemp = std::filesystem::temp_directory_path(ec);
	CHECK(!ec);
	CHECK(temp.Path().string().rfind(systemTemp.string(), 0) == 0);

	// The root is whatever directory the provider is handed, which is why no
	// fixed installation path appears anywhere in the provider.
	CHECK(temp.WriteText("any/where.bin", "PORTABLE"));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());
	CHECK(provider.GetResolvedRoot().is_absolute());

	const auto loaded = provider.Load(MakeId("any/where.bin"));
	CHECK(loaded.IsOk());
	CHECK_EQ(loaded.GetValue().AsStringView(), "PORTABLE");

	// A second provider over a different root is a different namespace, with no
	// shared or remembered state between them.
	const std::filesystem::path otherRoot = temp.Base() / "other";
	std::filesystem::create_directories(otherRoot, ec);
	CHECK(!ec);
	CHECK(temp.WriteOutside("other/any/where.bin", std::vector<uint8_t>{ 'O', 'T', 'H', 'E', 'R' }));

	FileSystemResourceProvider other(otherRoot);
	CHECK(other.Initialize().IsOk());

	const auto otherLoaded = other.Load(MakeId("any/where.bin"));
	CHECK(otherLoaded.IsOk());
	CHECK_EQ(otherLoaded.GetValue().AsStringView(), "OTHER");

	// The first provider still answers for its own root.
	CHECK_EQ(provider.Load(MakeId("any/where.bin")).GetValue().AsStringView(), "PORTABLE");
}

MODERN_TEST(ClientFilesystem_NoLegacyHeadersRequired)
{
	// The guarantee is structural: this translation unit includes TestHarness.h,
	// the modern client resource headers and standard headers only, and links
	// ModernClientResources and Modern, which link no legacy library, no
	// DirectX and no MFC. A legacy dependency would fail to compile or link.
	//
	// What can be asserted here is the contract the provider claims to satisfy:
	// it is a complete IResourceProvider, usable entirely through the interface,
	// and it is a leaf class rather than an extension point.
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.MakeDirectory("ui"));
	CHECK(temp.WriteText("ui/login.bin", "LOGIN"));

	FileSystemResourceProvider provider(temp.Path());
	CHECK(provider.Initialize().IsOk());

	IResourceProvider& asInterface = provider;
	const ResourceId id = MakeId("ui/login.bin");
	CHECK(asInterface.HasResource(id));

	const auto loaded = asInterface.Load(id);
	CHECK(loaded.IsOk());
	CHECK_EQ(loaded.GetValue().GetSize(), 5u);

	static_assert(std::is_base_of<IResourceProvider, FileSystemResourceProvider>::value,
		"FileSystemResourceProvider must satisfy IResourceProvider");
	static_assert(std::is_final<FileSystemResourceProvider>::value,
		"FileSystemResourceProvider is a leaf provider, not a base class");
}

MODERN_TEST(ClientFilesystem_UninitializedProviderIsDeterministic)
{
	TempRoot temp;
	CHECK(temp.IsUsable());
	CHECK(temp.WriteText("ui/login.bin", "LOGIN"));

	FileSystemResourceProvider provider(temp.Path());
	const ResourceId id = MakeId("ui/login.bin");

	// Before Initialize() there is no resolved root, so every question is
	// answered with a no or an error - never by guessing a directory.
	CHECK(!provider.IsInitialized());
	CHECK(provider.GetResolvedRoot().empty());
	CHECK(!provider.HasResource(id));
	CHECK_EQ(provider.Load(id).GetError(), ErrorCode::InvalidState);

	// After Initialize() the same identifier resolves.
	CHECK(provider.Initialize().IsOk());
	CHECK(provider.HasResource(id));
	CHECK(provider.Load(id).IsOk());
}

// ---------------------------------------------------------------------------
// Main Test Runner
// ---------------------------------------------------------------------------

int main()
{
	std::printf("Modern client resource boundary tests (CLIENT-005, CLIENT-006)\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n", static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
		failedCases,
		static_cast<int>(ModernTests::Registry().size()),
		ModernTests::FailureCount());
	return 1;
}

