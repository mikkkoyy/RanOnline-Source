#include "navigation/WldNavigationLoader.h"

#include "navigation/WldCrypt.h"

#include <filesystem>
#include <fstream>
#include <system_error>

namespace Modern::Navigation
{
	namespace
	{
		// A `.wld` is at least the 132-byte header, and the smallest shipped one is
		// far larger, so anything under this is refused before it is read at all.
		constexpr std::size_t kMinimumFileBytes = WldCrypt::kBodyStart;

		// The largest file this reader will pull into memory.
		//
		// Not a guess about map size: `deasindong_05.wld` is 34.7 MB, and the whole
		// asset set has to be loadable. 512 MB leaves headroom for an unreleased
		// map while still refusing a corrupt size field that would otherwise ask
		// for a multi-gigabyte allocation.
		constexpr std::size_t kMaximumFileBytes = 512u * 1024u * 1024u;

		NavigationLoadResult Fail(NavigationLoadResult result, NavigationLoadStatus status,
		                          const char* stage, const char* detail)
		{
			result.status = status;
			result.error.status = status;
			result.error.stage  = stage;
			result.error.detail = detail;
			result.error.hasOffset = false;
			return result;
		}
	}

	NavigationLoadResult BuildNavigationMeshFromBuffer(const std::uint8_t* data,
	                                                   std::size_t size)
	{
		NavigationLoadResult result;

		if (data == nullptr || size < kMinimumFileBytes)
		{
			return Fail(result, NavigationLoadStatus::InvalidFile, "header",
			            "buffer is smaller than the 132-byte header");
		}

		if (size > kMaximumFileBytes)
		{
			return Fail(result, NavigationLoadStatus::InvalidFile, "header",
			            "buffer is larger than the 512 MB load ceiling");
		}

		// An owned copy. The caller's bytes are never modified: decryption is a
		// destructive in-place transform, and mutating a buffer a caller still
		// holds would be a nasty surprise.
		std::vector<std::uint8_t> buffer(data, data + size);

		const bool encrypted = WldCrypt::IsEncrypted(buffer.data(), buffer.size());

		// A plain file is parsed as-is. An encrypted one is decrypted repeatedly
		// until the parse succeeds - see the note on the double-encrypted fixture
		// below.
		const int firstPass = encrypted ? 1 : 0;
		const int lastPass  = encrypted ? kMaxDecryptPasses : 0;

		NavigationLoadStatus lastStatus = NavigationLoadStatus::InvalidFile;
		WldNavigationError  lastError{};

		for (int pass = firstPass; pass <= lastPass; ++pass)
		{
			if (pass > 0)
			{
				WldCrypt::DecryptBody(buffer);
			}

			WldNavigationData   parsed;
			WldNavigationError  error;
			if (ParseWldNavigation(buffer.data(), buffer.size(), parsed, error))
			{
				// `ParseWldNavigation` returns true for `NoNavigation` as well as
				// `Loaded`, because both are successful reads of a valid file. The
				// distinction has to be made HERE: trying to build a mesh out of a
				// file with no cells fails on a null cell pointer, and reporting that
				// as a corrupt file would turn nine perfectly good login maps into
				// nine load errors.
				if (error.status == NavigationLoadStatus::NoNavigation)
				{
					result.status        = NavigationLoadStatus::NoNavigation;
					result.error         = error;
					result.decryptPasses = static_cast<std::uint32_t>(pass);
					result.encrypted     = encrypted;
					return result;
				}

				auto mesh = std::make_shared<NavigationMesh>();

				if (!mesh->Build(parsed.vertices, parsed.cellRecords.data(), parsed.CellCount(),
				                 parsed.linkIds))
				{
					// The parse already proved the cell ids equal their indices and
					// that every vertex and link resolves, so a failure here means
					// the tree could not be built over a set that parsed - which is a
					// kernel-level inconsistency and worth saying so plainly rather
					// than reporting as a bad file.
					result.status = NavigationLoadStatus::Inconsistent;
					result.error.status = NavigationLoadStatus::Inconsistent;
					result.error.stage  = "mesh build";
					result.error.detail = "NavigationMesh::Build rejected a block that parsed";
					result.error.hasOffset = false;
					return result;
				}

				result.status        = NavigationLoadStatus::Loaded;
				result.mesh          = std::move(mesh);
				result.decryptPasses = static_cast<std::uint32_t>(pass);
				result.encrypted     = encrypted;
				result.error.status  = NavigationLoadStatus::Loaded;
				result.error.stage   = "navigation";
				result.error.detail  = "navigation mesh built";
				result.error.hasOffset = false;
				return result;
			}

			lastStatus = error.status;
			lastError  = error;

			if (!encrypted)
			{
				break;
			}
		}

		// ------------------------------------------------------------------------
		// Why several passes, and why the loop tests the PARSE rather than the bytes
		// ------------------------------------------------------------------------
		//
		// `es_f41_killbillzone01.wld` is encrypted twice. One pass produces a
		// plausible-looking header - because `Encryption_WLD` writes the file-type
		// field in the clear for `i < 128` and leaves 128..131 alone, so a
		// single pass yields a correct `LAND.MAN` tag AND a correct FileID. What
		// is still wrong is the body, and the first thing that notices is the FILE
		// MARK: its version DWORD decodes to 0x20202120 - four ASCII spaces - which
		// is neither 0x0101 nor 0x0100, so `SLAND_FILEMARK::LoadSet` gives up
		// (DxLandDef.cpp:46-50). Two passes give 0x0100 and a clean parse.
		//
		// Legacy has NO equivalent of this loop. `CSerialFile` decrypts each read
		// chunk exactly once (SerialFile.cpp:232), so legacy reads this file
		// incorrectly and this loader does not. That is an improvement over legacy
		// and is recorded as one, not smuggled in as a format rule: the pass count
		// is reported on the result so a caller can see it happened.
		//
		// The loop is bounded by `kMaxDecryptPasses`, and it retries on ANY parse
		// failure - not on a byte-pattern guess. Testing the parse is what makes it
		// safe: a file that is not encrypted at all fails immediately at pass 0 and
		// never enters the loop, and an encrypted file with a corrupt body exhausts
		// four passes and reports a failure instead of spinning.
		if (encrypted)
		{
			// The file announced itself encrypted and no pass produced a usable parse.
			// Report the LAST attempt's reason, which is the most-decoded state.
			result.status = NavigationLoadStatus::DecryptionFailed;
			result.error.status = NavigationLoadStatus::DecryptionFailed;
			result.error.stage  = "decrypt";
			result.error.detail = "no decryption pass produced a readable file mark";
			result.error.hasOffset = false;
			return result;
		}

		result.status = lastStatus;
		result.error = lastError;
		return result;
	}

	NavigationLoadResult LoadWldNavigationMesh(const std::string& path)
	{
		NavigationLoadResult result;

		std::error_code ec;
		const auto      fileSize = std::filesystem::file_size(path, ec);
		if (ec)
		{
			return Fail(result, NavigationLoadStatus::InvalidFile, "open",
			            "file could not be opened");
		}

		if (fileSize < kMinimumFileBytes)
		{
			return Fail(result, NavigationLoadStatus::InvalidFile, "header",
			            "file is smaller than the 132-byte header");
		}

		if (fileSize > kMaximumFileBytes)
		{
			return Fail(result, NavigationLoadStatus::InvalidFile, "header",
			            "file is larger than the 512 MB load ceiling");
		}

		// Opened read-only and never written back. RAN's own assets are not
		// modified by this loader in any code path - the decrypted form exists
		// only in the local buffer.
		std::ifstream file(path, std::ios::binary);
		if (!file)
		{
			return Fail(result, NavigationLoadStatus::InvalidFile, "open",
			            "file could not be opened for reading");
		}

		std::vector<std::uint8_t> bytes(static_cast<std::size_t>(fileSize));
		file.read(reinterpret_cast<char*>(bytes.data()),
		          static_cast<std::streamsize>(bytes.size()));

		// A short read is a truncated file, not a zero-filled tail: report it rather
		// than letting the parser blame the navigation block for it.
		if (static_cast<std::size_t>(file.gcount()) != bytes.size())
		{
			return Fail(result, NavigationLoadStatus::Truncated, "read",
			            "file ended before the reported size");
		}

		result = BuildNavigationMeshFromBuffer(bytes.data(), bytes.size());
		if (!result.Ok())
		{
			// The parser does not know the file's name and its messages are meant
			// to be short; the name belongs on the log line, not in every message.
			result.error.detail = path + ": " + result.error.detail;
		}
		return result;
	}
}