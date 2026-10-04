#include "LoginServerConfig.h"

namespace Modern::Server
{
	using Network::GameServerInfo;

	Status LoginServerConfig::Validate() const
	{
		// Refused rather than defaulted. Legacy substitutes INADDR_ANY for an
		// unparsable server_ip (s_CServer.cpp:627-631); reproducing that would mean a
		// typo in a config file quietly turns a loopback-only server into one exposed
		// to every interface, and the symptom - clients that should not have connected
		// connecting - appears nowhere near the mistake.
		if (bind.host.empty())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// "localhost" is allowed because it names nothing but this machine and the
		// transport resolves it. Any OTHER name is refused even though the transport
		// could resolve it: the RAN convention is a numeric address in configuration,
		// and a bind address nobody can read off the server's own configuration is a
		// bad bind address. Note this is a rule about CONFIGURATION, not about TCP -
		// SocketAddress.h explains why the two are compatible.
		if (!Network::EndpointAddress::IsNumericIPv4(bind.host) && bind.host != "localhost")
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Port 0 is legal and means "OS-assigned". A port above the wire's range is
		// refused rather than truncated, because a silently narrowed port connects to a
		// different service than the operator asked for.
		if (bind.port > 65535)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// The grid is deliberately NOT checked for emptiness. An empty list is a real,
		// reportable state - see the header.
		return Ok();
	}

	namespace LoginServerFixture
	{
		namespace
		{
			GameServerInfo Make(const char* ip, Network::WireI32 port, Network::WireI32 group,
			                    Network::WireI32 number, Network::WireI32 clients,
			                    Network::WireI32 maxClients)
			{
				GameServerInfo info;
				info.ip             = ip;
				info.servicePort    = port;
				info.serverGroup    = group;
				info.serverNumber   = number;
				info.currentClients = clients;
				info.maxClients     = maxClients;
				info.pk             = Network::GameServerList::kDefaultPk;
				return info;
			}
		}

		Network::GameServerGrid Default()
		{
			Network::GameServerGrid grid;
			(void)grid.Add(Make("127.0.0.1", 5101, 1, 1, 3, 100));
			(void)grid.Add(Make("127.0.0.1", 5102, 1, 3, 0, 50));
			(void)grid.Add(Make("127.0.0.1", 5103, 2, 1, 17, 200));
			return grid;
		}

		Network::GameServerGrid Single()
		{
			Network::GameServerGrid grid;
			(void)grid.Add(Make("127.0.0.1", 5101, 0, 0, 0, 10));
			return grid;
		}

		Network::GameServerGrid Empty()
		{
			return Network::GameServerGrid();
		}
	}
}