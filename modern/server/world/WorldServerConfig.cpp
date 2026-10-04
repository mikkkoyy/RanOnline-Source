#include "world/WorldServerConfig.h"

namespace Modern::Server::World
{
	namespace
	{
		constexpr const char* kLoopback = "127.0.0.1";
	}

	Status WorldServerConfig::Validate() const
	{
		if (agentBind.host.empty() || fieldBind.host.empty())
		{
			// Refused rather than defaulted. A listener bound to "0.0.0.0" is a server
			// reachable from the network, which is not a decision a config default
			// should make on the operator's behalf.
			return Status(ErrorCode::InvalidArgument);
		}

		if (agentBind.port == fieldBind.port && agentBind.port != 0 &&
		    agentBind.host == fieldBind.host)
		{
			// Two roles on one port cannot both listen, and a runtime that bound them
			// onto the same endpoint would have silently merged the two roles the brief
			// requires to be separate.
			return Status(ErrorCode::InvalidArgument);
		}

		// No duplicate login names, and no two logins sharing an account id.
		//
		// Either would make "which account did this session authenticate as" depend on
		// table order - and that answer decides which characters the session may see,
		// so an order-dependent answer is an ownership bug waiting to happen.
		for (std::size_t i = 0; i < accounts.size(); ++i)
		{
			if (accounts[i].userId.empty() || accounts[i].accountId.value == 0)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			for (std::size_t j = i + 1; j < accounts.size(); ++j)
			{
				if (accounts[i].userId == accounts[j].userId)
				{
					return Status(ErrorCode::InvalidArgument);
				}
				if (accounts[i].accountId == accounts[j].accountId)
				{
					return Status(ErrorCode::InvalidArgument);
				}
			}
		}

		return Ok();
	}

	const WorldAccountBinding* WorldServerConfig::FindAccount(
	    const std::string& userId) const noexcept
	{
		for (const WorldAccountBinding& binding : accounts)
		{
			if (binding.userId == userId)
			{
				return &binding;
			}
		}
		return nullptr;
	}

	WorldServerConfig WorldServerConfig::MakeLoopback()
	{
		WorldServerConfig config;
		config.agentBind.host = kLoopback;
		config.agentBind.port = 0;  // OS-assigned
		config.fieldBind.host = kLoopback;
		config.fieldBind.port = 0;  // OS-assigned
		return config;
	}
}