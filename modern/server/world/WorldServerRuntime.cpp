		#include "world/WorldServerRuntime.h"

#include "item/ItemDefinitionTable.h"
#include "skills/SkillBasicTable.h"

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
		// WORLD-ENTRY-002L-C: bind the recovered provider to the live combat path.
		//
		// 002L-B deliberately left this unbound and recorded the measured
		// consequence of binding it - the suite's level-10 ArcherMale attacker
		// derives range (10,10), its level-30 target derives defence 10, and the
		// hit crits for 12. This milestone owns that consequence, so the provider
		// is installed and the wire figure moves from the prototype 10-20 band to
		// the recovered one.
		//
		// The runtime does NOT adopt its state: the provider is stateless and reads
		// `Stats::ClassConstantTable::Verified()`, so there is no second model of
		// anything here.
		m_field.SetCombatStatsProvider(&m_combatStats);

		// WORLD-ENTRY-002M: the item table, loaded from the configured path.
		//
		// A path that is set but unreadable leaves the table empty and is reported
		// through the counter, so a deployment with a stale path sees "no
		// equipment effects" rather than a silently half-loaded table. Nothing is
		// fatal here: an empty table is a VALID configuration, and every existing
		// test runs with one.
		if (!m_config.itemDataPath.empty())
		{
			const auto loaded = Item::LoadItemCsv(m_config.itemDataPath, m_itemTable);
			m_itemRowsLoaded =
			    loaded.IsOk() ? static_cast<std::size_t>(loaded.GetValue().loaded) : 0;
		}

		// Bound whether or not anything loaded: an empty provider is a state the
		// runtime can be in, and handing out a pointer to it is how the field role
		// and the combat provider learn that.
		m_combatStats.SetItemDefinitions(&m_itemTable);
		m_field.SetItemDefinitions(&m_itemTable);

		// SKILL-025: the skill table, loaded from the configured path.
		//
		// An empty path loads nothing and is VALID - the table stays empty, the
		// provider below is still bound, and every cast is answered from that
		// honest "no skill definitions are known" state. That is what every
		// existing test runs.
		//
		// A path that is set but unreadable is NOT valid: the failure is
		// recorded here (the constructor cannot fail) and enforced in Start(),
		// which refuses before either role binds a port. Serving casts against
		// definitions the deployment named but the server does not have would
		// be the silent half-load this milestone exists to prevent.
		if (!m_config.skillDataPath.empty())
		{
			const auto loaded =
			    Modern::Skill::LoadSkillCsv(m_config.skillDataPath, m_skillTable);
			if (loaded.IsOk())
			{
				m_skillRowsLoaded = static_cast<std::size_t>(loaded.GetValue().loaded);
			}
			else
			{
				m_skillRowsLoaded  = 0;
				m_skillLoadStatus = loaded.GetStatus();
			}
		}

		// Bound whether or not anything loaded, exactly as the item table is:
		// the field role borrows this pointer for every session it creates, so
		// the provider is never silently null - only ever empty.
		m_field.SetSkillDefinitions(&m_skillTable);
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

		// SKILL-025: a configured skill path that failed to load refuses the
		// whole start, before either role binds a port. The constructor recorded
		// the failure; this is where it is enforced, because Start() is the
		// operation that can fail and the constructor is not. An operator who
		// named skill data gets "the server would not start and said why",
		// never "the server runs and knows no skills".
		if (m_skillLoadStatus.IsError())
		{
			return m_skillLoadStatus;
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
