#include "resources/FileSystemResourceProvider.h"

#include <cstdint>
#include <fstream>
#include <ios>
#include <limits>
#include <system_error>
#include <utility>
#include <vector>

namespace Modern::Client
{
namespace
{
	// A logical identifier has no reason to approach any filesystem path limit,
	// and a short bound keeps every later operation comfortably inside them.
	constexpr size_t kMaxLogicalNameLength = 512;

	bool IsPortableAscii(char c) noexcept
	{
		const unsigned char byte = static_cast<unsigned char>(c);

		// Control characters, NUL, DEL and every non-ASCII byte are refused.
		if (byte < 0x20 || byte > 0x7E)
		{
			return false;
		}

		// ':' introduces a drive letter or an NTFS alternate data stream;
		// '\' is the Windows separator and the start of a UNC or device path.
		return c != ':' && c != '\\';
	}

	bool EndsWithDotOrSpace(std::string_view component) noexcept
	{
		const char last = component.back();
		return last == '.' || last == ' ';
	}
}

FileSystemResourceProvider::FileSystemResourceProvider(std::filesystem::path rootDirectory)
	: m_root(std::move(rootDirectory))
{
}

bool FileSystemResourceProvider::IsPortableLogicalName(std::string_view name) noexcept
{
	if (name.empty() || name.size() > kMaxLogicalNameLength)
	{
		return false;
	}

	// A leading separator, or a trailing one, would produce an empty first or
	// last component. ResourceId::Create() already refuses both; re-checking
	// here keeps the guarantee next to the code that touches the filesystem.
	if (name.front() == '/' || name.front() == '\\' || name.back() == '/')
	{
		return false;
	}

	size_t componentStart = 0;
	while (componentStart < name.size())
	{
		const size_t separator = name.find('/', componentStart);
		const size_t componentEnd = (separator == std::string_view::npos) ? name.size() : separator;

		const std::string_view component = name.substr(componentStart, componentEnd - componentStart);

		// "a//b" and "a/" both land here: an empty component is a separator
		// alias rather than an addressable name, and some systems read the
		// leading separator of "//server" as a root name.
		if (component.empty())
		{
			return false;
		}

		if (component == "." || component == "..")
		{
			// Traversal, refused lexically. ".." has no meaning in a logical
			// asset namespace, so there is nothing to normalise.
			return false;
		}

		if (EndsWithDotOrSpace(component))
		{
			// Windows silently strips a trailing dot or space, so "a" and "a."
			// would open one file while comparing as two identifiers.
			return false;
		}

		for (const char c : component)
		{
			if (!IsPortableAscii(c))
			{
				return false;
			}
		}

		if (separator == std::string_view::npos)
		{
			break;
		}

		componentStart = separator + 1;
	}

	return true;
}

Status FileSystemResourceProvider::Initialize()
{
	if (m_initialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	if (m_root.empty())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	std::error_code ec;
	const std::filesystem::file_status rootStatus = std::filesystem::status(m_root, ec);
	if (ec)
	{
		// status()'s error-code overload is the non-throwing one, and a missing
		// root is the case worth naming: the caller has a path to fix. Anything
		// else (a denied directory, an unavailable drive) is also unusable as a
		// root, but the identifier itself was fine.
		if (ec == std::errc::no_such_file_or_directory)
		{
			return Status(ErrorCode::NotFound);
		}

		return Status(ErrorCode::InvalidArgument);
	}

	if (!std::filesystem::exists(rootStatus))
	{
		// Some implementations report a missing path as not_found rather than
		// as an error code. Both mean the same thing here.
		return Status(ErrorCode::NotFound);
	}

	if (!std::filesystem::is_directory(rootStatus))
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// canonical() needs the root to exist, which the checks above establish. It
	// also resolves symlinks, so containment can be compared component-wise
	// against a stable absolute prefix from here on.
	std::filesystem::path resolved = std::filesystem::canonical(m_root, ec);
	if (ec || resolved.empty())
	{
		return Status(ErrorCode::NotFound);
	}

	m_resolvedRoot = std::move(resolved);
	m_initialized = true;
	return Ok();
}

bool FileSystemResourceProvider::IsInsideRoot(const std::filesystem::path& candidate) const noexcept
{
	auto rootIt = m_resolvedRoot.begin();
	auto rootEnd = m_resolvedRoot.end();
	auto candidateIt = candidate.begin();

	for (; rootIt != rootEnd; ++rootIt, ++candidateIt)
	{
		if (candidateIt == candidate.end())
		{
			return false;
		}

		if (*rootIt != *candidateIt)
		{
			return false;
		}
	}

	// Strictly below the root: an identifier never addresses the root itself.
	return candidateIt != candidate.end();
}

Result<std::filesystem::path> FileSystemResourceProvider::ResolvePath(const ResourceId& id) const
{
	if (!m_initialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	if (!id.IsValid())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	const std::string_view name = id.GetView();
	if (!IsPortableLogicalName(name))
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// Built from an already-validated, ASCII-only, separator-checked namespace,
	// so there is no encoding conversion to perform and no root name to pick up.
	const std::filesystem::path logical(name);
	if (logical.is_absolute() || logical.has_root_path())
	{
		// Unreachable while IsPortableLogicalName() holds: a relative path made
		// of plain components has neither. It stays as the last lexical guard,
		// because everything after this point hands the path to the filesystem.
		return Status(ErrorCode::InvalidArgument);
	}

	const std::filesystem::path candidate = m_resolvedRoot / logical;

	// Semantic containment, the half the lexical rules cannot do. Canonicalising
	// resolves the components that exist - including symlinks, which no naming
	// rule can see through - and normalises the rest, so a link inside the root
	// pointing outside it fails here even though every component name is
	// innocent. weakly_canonical() rather than canonical() because the target
	// file need not exist yet: HasResource() must be able to answer no.
	std::error_code ec;
	const std::filesystem::path resolved = std::filesystem::weakly_canonical(candidate, ec);
	if (ec)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	if (!IsInsideRoot(resolved))
	{
		return Status(ErrorCode::InvalidArgument);
	}

	return resolved;
}

bool FileSystemResourceProvider::HasResource(const ResourceId& id) const
{
	const Result<std::filesystem::path> resolved = ResolvePath(id);
	if (resolved.IsError())
	{
		return false;
	}

	std::error_code ec;
	const std::filesystem::file_status fileStatus = std::filesystem::status(resolved.GetValue(), ec);
	if (ec)
	{
		return false;
	}

	// is_regular_file() rather than exists(): directories, device names and
	// reparse points to either are not resources. The stream is never opened, so
	// HasResource() answers from metadata alone.
	return std::filesystem::is_regular_file(fileStatus);
}

Result<ResourceData> FileSystemResourceProvider::Load(const ResourceId& id)
{
	const Result<std::filesystem::path> resolved = ResolvePath(id);
	if (resolved.IsError())
	{
		return resolved.GetStatus();
	}

	const std::filesystem::path& path = resolved.GetValue();

	std::error_code ec;
	const std::filesystem::file_status fileStatus = std::filesystem::status(path, ec);
	if (ec || !std::filesystem::is_regular_file(fileStatus))
	{
		return Status(ErrorCode::NotFound);
	}

	// Open at the end so the byte count is known before anything is transferred.
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
	{
		return Status(ErrorCode::NotFound);
	}

	const std::streamoff end = file.tellg();
	if (end < 0)
	{
		return Status(ErrorCode::NotFound);
	}

	// A size no size_t can hold would truncate the buffer on a 32-bit build, and
	// returning half a resource silently is worse than refusing it.
	if (static_cast<std::uintmax_t>(end) > static_cast<std::uintmax_t>(std::numeric_limits<size_t>::max()))
	{
		return Status(ErrorCode::NotFound);
	}

	std::vector<uint8_t> bytes(static_cast<size_t>(end));
	if (!bytes.empty())
	{
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

		if (!file)
		{
			// A regular file that could not be read to the end: a share
			// violation, a truncated file, a device that lied about its size.
			// The core error vocabulary has no dedicated I/O code by design, so
			// this reuses NotFound - in both cases the provider cannot produce
			// the resource, and adding an I/O code is a Core-level decision.
			return Status(ErrorCode::NotFound);
		}
	}

	// An empty file is a successful load of zero bytes: a value, not an error.
	return ResourceData(std::move(bytes));
}

} // namespace Modern::Client
