#include "ResourceSyncService.h"
#include "UpdateStateProtocol.h"

namespace Modern::Server::World
{
    using Network::WireU8;
    using Network::WireU32;

    Status ResourceSyncService::RegisterSession(const WorldCharacter& character,
                                                 SelfFrameSink self,
                                                 HpFrameSink broadcast,
                                                 WriteBackSink writeBack)
    {
        const WireU32 gaeaId = character.gaeaId;
        if (gaeaId == 0)
        {
            return Status(ErrorCode::InvalidArgument);
        }

        if (self == nullptr || broadcast == nullptr || writeBack == nullptr)
        {
            return Status(ErrorCode::InvalidArgument);
        }

        Modern::Resources::ResourceState::ResourceRecord record;

        // Field-named, and deliberately NOT brace-initialised from the DwPair.
        //
        // The two types have OPPOSITE field order:
        //     RanWire::DwPair                = { now,  max   }
        //     Resources::Pool                = { maximum, current }
        //
        // so `record.hp = { character.hp.now, character.hp.max }` compiles, reads
        // as obvious, and quietly puts the CURRENT value in the MAXIMUM slot. The
        // effect is not a cosmetic slip: the character's real maximum is lost, so
        // recovery accrues against the wounded value, Restore clamps at the wounded
        // value - which makes damage permanent - and every 3046 tells the client a
        // maximum that is not the character's.
        record.hp.maximum = character.hp.max;
        record.hp.current = character.hp.now;
        record.mp.maximum = character.mp.max;
        record.mp.current = character.mp.now;
        record.sp.maximum = character.sp.max;
        record.sp.current = character.sp.now;

        // Legacy globals - no items, passives or facts in the world model
        // yet. StatCalculator mirrors these:
        //   kHp = 0.3f * 0.01f = 0.003f
        //   kMp = 0.003f
        //   kSp = 0.005f
        //   all flats = 0
        record.hpRecovery = { Stats::RecoveryRateConstant::kHp,
                              Stats::RecoveryFlatConstant::kHp, 0.0f };
        record.mpRecovery = { Stats::RecoveryRateConstant::kMp,
                              Stats::RecoveryFlatConstant::kMp, 0.0f };
        record.spRecovery = { Stats::RecoveryRateConstant::kSp,
                              Stats::RecoveryFlatConstant::kSp, 0.0f };

        const auto adoptResult = Modern::Resources::ResourceState::Adopt(record);
        if (adoptResult.IsError())
        {
            return adoptResult.GetStatus();
        }

        Session session;
        session.gaeaId       = gaeaId;
        session.characterId  = character.id.value;
        session.name         = character.name;
        session.state        = std::move(adoptResult.GetValue());
        session.stateTimer   = 0.0f;
        session.selfSink     = std::move(self);
        session.hpSink       = std::move(broadcast);
        session.writeBackSink = std::move(writeBack);

        std::lock_guard lock(m_mutex);
        if (m_sessions.find(gaeaId) != m_sessions.end())
        {
            return Status(ErrorCode::InvalidArgument);
        }
        m_sessions.emplace(gaeaId, std::move(session));
        return Ok();
    }

    Status ResourceSyncService::UnregisterSession(WireU32 gaeaId)
    {
        std::lock_guard lock(m_mutex);
        const auto it = m_sessions.find(gaeaId);
        if (it == m_sessions.end())
        {
            return Status(ErrorCode::NotFound);
        }
        m_sessions.erase(it);
        return Ok();
    }

    void ResourceSyncService::Advance(float elapsedSeconds)
    {
        if (!std::isfinite(elapsedSeconds) || elapsedSeconds <= 0.0f)
        {
            return;
        }

        struct PendingSelf
        {
            SelfFrameSink sink;
            std::vector<WireU8> frame;
            WriteBackSink writeBack;
            WireU32 hpNow, hpMax, mpNow, mpMax, spNow, spMax;
        };
        struct PendingHp
        {
            HpFrameSink sink;
            std::vector<WireU8> frame;
        };

        std::vector<PendingSelf> selfQueue;
        std::vector<PendingHp>   hpQueue;

        {
            std::lock_guard lock(m_mutex);
            for (auto& [id, session] : m_sessions)
            {
                // Dead gate: GLChar.cpp:5853 - skip UPDATE_DATA and
                // UpdateClientState entirely while dwNow is 0.
                if (session.state.GetCurrent(Resources::ResourceKind::Hp) == 0)
                {
                    continue;
                }

                // Recovery (ResourceState::Recover validates elapsed > 0;
                // we already checked, but keep the call safe).
                (void)session.state.Recover(elapsedSeconds);

                // State timer - legacy resets to 0.0f, does NOT subtract.
                session.stateTimer += elapsedSeconds;
                if (session.stateTimer > kStateTimerSeconds)
                {
                    session.stateTimer = 0.0f;

                    std::vector<WireU8> frame;
                    const WireU32 hpNow = session.state.GetCurrent(Resources::ResourceKind::Hp);
                    const WireU32 hpMax = session.state.GetMaximum(Resources::ResourceKind::Hp);
                    const WireU32 mpNow = session.state.GetCurrent(Resources::ResourceKind::Mp);
                    const WireU32 mpMax = session.state.GetMaximum(Resources::ResourceKind::Mp);
                    const WireU32 spNow = session.state.GetCurrent(Resources::ResourceKind::Sp);
                    const WireU32 spMax = session.state.GetMaximum(Resources::ResourceKind::Sp);

                    if (const Status st = Network::UpdateState::UpdateStateCodec::AppendStateUpdate(frame,
                        {
                            { hpNow, hpMax },
                            { mpNow, mpMax },
                            { spNow, spMax },
                            { 0, 0 },
                            session.name,
                            session.gaeaId,
                            session.characterId,
                            false
                        });
                        st.IsOk())
                    {
                        selfQueue.push_back({ session.selfSink,
                                              std::move(frame),
                                              session.writeBackSink,
                                              hpNow, hpMax, mpNow, mpMax, spNow, spMax });
                    }
                }
            }
        }

        // Invoke sinks outside the mutex.
        for (auto& p : selfQueue)
        {
            p.sink(p.frame);
            p.writeBack(
                { p.hpNow, p.hpMax },
                { p.mpNow, p.mpMax },
                { p.spNow, p.spMax });
        }

        // hpQueue is unused in Advance (no broadcast on timer).
        (void)hpQueue;
    }

    // -------------------------------------------------------------------------
    // Frame construction and delivery.
    //
    // Both require m_mutex held and touch no I/O; Deliver takes neither the lock
    // nor any reference to a Session.
    // -------------------------------------------------------------------------

    void ResourceSyncService::BuildSelfFrame(const Session& session, Outgoing& out)
    {
        std::vector<WireU8> frame;
        const Status st = Network::UpdateState::UpdateStateCodec::AppendStateUpdate(
            frame,
            {
                { session.state.GetCurrent(Resources::ResourceKind::Hp),
                  session.state.GetMaximum(Resources::ResourceKind::Hp) },
                { session.state.GetCurrent(Resources::ResourceKind::Mp),
                  session.state.GetMaximum(Resources::ResourceKind::Mp) },
                { session.state.GetCurrent(Resources::ResourceKind::Sp),
                  session.state.GetMaximum(Resources::ResourceKind::Sp) },
                { 0, 0 },
                session.name,
                session.gaeaId,
                session.characterId,
                false
            });
        if (st.IsError())
        {
            return;
        }

        out.hasSelf   = true;
        out.self      = session.selfSink;
        out.selfFrame = std::move(frame);
        out.writeBack = session.writeBackSink;
        out.hp        = { session.state.GetCurrent(Resources::ResourceKind::Hp),
                          session.state.GetMaximum(Resources::ResourceKind::Hp) };
        out.mp        = { session.state.GetCurrent(Resources::ResourceKind::Mp),
                          session.state.GetMaximum(Resources::ResourceKind::Mp) };
        out.sp        = { session.state.GetCurrent(Resources::ResourceKind::Sp),
                          session.state.GetMaximum(Resources::ResourceKind::Sp) };
    }

    void ResourceSyncService::BuildHpFrame(const Session& session, Outgoing& out)
    {
        std::vector<WireU8> frame;
        const Status st = Network::UpdateState::UpdateStateCodec::AppendStateBroadcast(
            frame,
            {
                session.gaeaId,
                { session.state.GetCurrent(Resources::ResourceKind::Hp),
                  session.state.GetMaximum(Resources::ResourceKind::Hp) },
                false
            });
        if (st.IsError())
        {
            return;
        }

        out.hasHp   = true;
        out.hpSink  = session.hpSink;
        out.hpFrame = std::move(frame);
    }

    void ResourceSyncService::Deliver(const Outgoing& out)
    {
        // Self first, then the write-back, then the broadcast. The write-back
        // updates the repository and the broadcast tells everyone else, so a
        // listener that reacts to a 3053 by reading the character must not be
        // able to observe it before the record behind it has been updated.
        if (out.hasSelf)
        {
            out.self(out.selfFrame);
            out.writeBack(out.hp, out.mp, out.sp);
        }
        if (out.hasHp)
        {
            out.hpSink(out.hpFrame);
        }
    }

    Status ResourceSyncService::Spend(WireU32 gaeaId,
                                       Resources::ResourceKind kind,
                                       WireU32 amount,
                                       WireU32& spent)
    {
        spent = 0;

        if (amount == 0)
        {
            return Ok();
        }

        // ONE lock acquisition, held across the lookup, the mutation and the
        // frame encoding. The previous shape - find under the lock, release it,
        // then mutate through the raw pointer - let UnregisterSession erase the
        // map entry and rebalance the tree while this thread was still reading
        // and writing through the pointer it had obtained.
        Outgoing out;
        {
            const std::lock_guard lock(m_mutex);

            const auto it = m_sessions.find(gaeaId);
            if (it == m_sessions.end())
            {
                return Status(ErrorCode::NotFound);
            }
            Session& session = it->second;

            if (session.state.GetCurrent(kind) == 0)
            {
                return Ok();
            }

            const WireU32 actual = session.state.Spend(kind, amount);
            if (actual == 0)
            {
                return Ok();
            }

            spent = actual;

            BuildSelfFrame(session, out);
            if (kind == Resources::ResourceKind::Hp)
            {
                BuildHpFrame(session, out);
            }
        }

        Deliver(out);
        return Ok();
    }

    Status ResourceSyncService::Restore(WireU32 gaeaId,
                                         Resources::ResourceKind kind,
                                         WireU32 amount)
    {
        if (amount == 0)
        {
            return Ok();
        }

        Outgoing out;
        {
            const std::lock_guard lock(m_mutex);

            const auto it = m_sessions.find(gaeaId);
            if (it == m_sessions.end())
            {
                return Status(ErrorCode::NotFound);
            }
            Session& session = it->second;

            const WireU32 before = session.state.GetCurrent(kind);
            session.state.Restore(kind, amount);
            if (before == session.state.GetCurrent(kind))
            {
                return Ok();
            }

            BuildSelfFrame(session, out);
            if (kind == Resources::ResourceKind::Hp)
            {
                BuildHpFrame(session, out);
            }
        }

        Deliver(out);
        return Ok();
    }

    WireU32 ResourceSyncService::ApplyDamage(WireU32 gaeaId, WireU32 amount)
    {
        if (amount == 0)
        {
            return 0;
        }

        WireU32 applied = 0;
        Outgoing out;
        {
            const std::lock_guard lock(m_mutex);

            const auto it = m_sessions.find(gaeaId);
            if (it == m_sessions.end())
            {
                return 0;
            }
            Session& session = it->second;

            if (session.state.GetCurrent(Resources::ResourceKind::Hp) == 0)
            {
                return 0;
            }

            applied = session.state.ApplyDamage(amount);
            if (applied == 0)
            {
                return 0;
            }

            BuildSelfFrame(session, out);
            BuildHpFrame(session, out);
        }

        Deliver(out);
        return applied;
    }

    std::optional<Resources::ResourceState> ResourceSyncService::Find(WireU32 gaeaId) const
    {
        const std::lock_guard lock(m_mutex);
        const auto it = m_sessions.find(gaeaId);
        if (it == m_sessions.end())
        {
            return std::nullopt;
        }
        return it->second.state;
    }

    WireU32 ResourceSyncService::Current(WireU32 gaeaId,
                                          Resources::ResourceKind kind,
                                          WireU32 fallback) const
    {
        const std::lock_guard lock(m_mutex);
        const auto it = m_sessions.find(gaeaId);
        if (it == m_sessions.end())
        {
            return fallback;
        }
        return it->second.state.GetCurrent(kind);
    }

    std::size_t ResourceSyncService::SessionCount() const noexcept
    {
        const std::lock_guard lock(m_mutex);
        return m_sessions.size();
    }
}
