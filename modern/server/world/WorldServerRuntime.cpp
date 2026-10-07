		#include "world/WorldServerRuntime.h"

#include <utility>

namespace Modern::Server::World
{
	WorldServerRuntime::WorldServerRuntime(WorldServerConfig config,
	                                       ILoginAuthenticator& authenticator,
	                                       ICharacterRepository& repository)
		: m_config(std::move(config))
		, m_repository(repository)
		, m_agent(m_config, authenticator, repository, m_registry)
		, m_field(m_config, repository, m_registry, m_movement)
	{
	}

	WorldServerRuntime::~WorldServerRuntime()
	{
		Stop();
	}

	Status WorldServerRuntime::Start()
	{
		if (m_running)
		{
			// Reported rather than silently ignored: a caller that believes it started a
			// second server and did not would then have two objects claiming one port.
			return Status(ErrorCode::InvalidState);
		}

		// The FIELD role binds first, and that ordering is load-bearing.
		//
		// The Agent's 2358 must name an endpoint that is genuinely listening, so the
		// Field's real port has to be known before the Agent can be told where to send
		// clients. Starting the Agent first would mean either advertising a requested
		// port that the OS may not have granted, or starting the Agent twice.
		if (const Status status = m_field.Start(); status.IsError())
		{
			return status;
		}

		// Publish the endpoint the OS actually assigned. With port 0 requested, this is
		// not the port that was asked for - and it is the only one a client's second
		// connection can reach.
		FieldEndpoint advertised;
		advertised.address     = m_field.FieldBoundEndpoint().host;
		advertised.servicePort = static_cast<Network::WireI32>(m_field.FieldBoundEndpoint().port);

		if (const Status status = m_agent.Start(); status.IsError())
		{
			// The Field is already listening. Stopping it here means a failed Start
			// leaves nothing behind - otherwise a retry would fail to bind the same
			// port, and the operator would be chasing a phantom "address in use".
// WORLD-ENTRY-002f: the movement ticker is stopped BEFORE the listeners. A ticker
		// running while the worker threads are being joined would keep advancing actors
		// for characters whose Field sessions have already gone, and the joined threads
		// would be waiting on a runtime that is still mutating.
		m_field.StopMovementTicker();
			m_field.Stop();
			return status;
		}

		// Remembered so a test can compare what the Agent advertises against what the
		// Field actually bound - which is the assertion that makes 2358 authoritative.
		m_fieldEndpoint = m_field.FieldBoundEndpoint();

		// Handed to the redirect builder AFTER both listeners exist, so the very first
		// 2358 this process can emit already names a live endpoint.
		m_agent.SetAdvertisedFieldEndpoint(advertised);

		m_running = true;
		return Ok();
	}

	void WorldServerRuntime::Stop() noexcept
	{
		// WORLD-ENTRY-002f: the movement ticker is stopped BEFORE the listeners. A ticker
		// running while the Field role's worker threads are being joined would keep
		// advancing actors for characters whose sessions have already gone, and the
		// joined threads would be waiting on a runtime that is still mutating.
		m_field.StopMovementTicker();

		// WORLD-ENTRY-002h: the resource ticker is stopped before the field role,
		// for the same reason - it must not advance resources for sessions that
		// have already been torn down.
		m_field.StopResourceTicker();

		m_agent.Stop();
		m_field.Stop();
		m_running = false;
	}

	bool WorldServerRuntime::IsRunning() const noexcept
	{
		return m_running;
	}

	Network::Endpoint WorldServerRuntime::AgentEndpoint() const noexcept
	{
		return m_agent.AgentEndpoint();
	}

	Network::Endpoint WorldServerRuntime::FieldBoundEndpoint() const noexcept
	{
		return m_field.FieldBoundEndpoint();
	}

	Status WorldServerRuntime::ServeOneAgentClient(int timeoutMilliseconds)
	{
		return m_agent.ServeOneConnection(timeoutMilliseconds);
	}

}
