#include "resources/ResourceId.h"

namespace Modern::Client
{

Result<ResourceId> ResourceId::Create(std::string_view name)
{
	if (name.empty())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// Reject whitespace-only names or names containing backslashes (must be canonical forward-slash).
	size_t firstNonWs = name.find_first_not_of(" \t\r\n");
	if (firstNonWs == std::string_view::npos)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	if (name.find('\\') != std::string_view::npos)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// Must not start or end with a slash
	if (name.front() == '/' || name.back() == '/')
	{
		return Status(ErrorCode::InvalidArgument);
	}

	return ResourceId(std::string(name));
}

} // namespace Modern::Client
