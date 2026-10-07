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
        record.hp = { character.hp.now, character.hp.max };
        record.mp = { character.mp.now, character.mp.max };
        record.sp = { character.sp.now, character.sp.max };

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

        Session* session = nullptr;
        {
            std::lock_guard lock(m_mutex);
            auto it = m_sessions.find(gaeaId);
            if (it == m_sessions.end())
            {
                return Status(ErrorCode::NotFound);
            }
            session = &it->second;
        }

        const WireU32 current = session->state.GetCurrent(kind);
        if (current == 0)
        {
            return Ok();
        }

        const WireU32 actual = session->state.Spend(kind, amount);
        if (actual == 0)
        {
            return Ok();
        }

        spent = actual;

        // Emit 3046 self
        {
            std::vector<WireU8> frame;
            const WireU32 hpNow = session->state.GetCurrent(Resources::ResourceKind::Hp);
            const WireU32 hpMax = session->state.GetMaximum(Resources::ResourceKind::Hp);
            const WireU32 mpNow = session->state.GetCurrent(Resources::ResourceKind::Mp);
            const WireU32 mpMax = session->state.GetMaximum(Resources::ResourceKind::Mp);
            const WireU32 spNow = session->state.GetCurrent(Resources::ResourceKind::Sp);
            const WireU32 spMax = session->state.GetMaximum(Resources::ResourceKind::Sp);

            if (const Status st = Network::UpdateState::UpdateStateCodec::AppendStateUpdate(frame,
                {
                    { hpNow, hpMax },
                    { mpNow, mpMax },
                    { spNow, spMax },
                    { 0, 0 },
                    session->name,
                    session->gaeaId,
                    session->characterId,
                    false
                });
                st.IsOk())
            {
                SelfFrameSink selfCopy = session->selfSink;
                WriteBackSink wbCopy   = session->writeBackSink;
                selfCopy(frame);
                wbCopy({ hpNow, hpMax }, { mpNow, mpMax }, { spNow, spMax });
            }
        }

        // HP spend also emits 3053
        if (kind == Resources::ResourceKind::Hp)
        {
            std::vector<WireU8> frame;
            const WireU32 hpNow = session->state.GetCurrent(Resources::ResourceKind::Hp);
            const WireU32 hpMax = session->state.GetMaximum(Resources::ResourceKind::Hp);

            if (const Status st = Network::UpdateState::UpdateStateCodec::AppendStateBroadcast(frame,
                {
                    session->gaeaId,
                    { hpNow, hpMax },
                    false
                });
                st.IsOk())
            {
                HpFrameSink hpCopy = session->hpSink;
                hpCopy(frame);
            }
        }

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

        Session* session = nullptr;
        {
            std::lock_guard lock(m_mutex);
            auto it = m_sessions.find(gaeaId);
            if (it == m_sessions.end())
            {
                return Status(ErrorCode::NotFound);
            }
            session = &it->second;
        }

        const WireU32 before = session->state.GetCurrent(kind);
        session->state.Restore(kind, amount);
        const WireU32 after = session->state.GetCurrent(kind);

        if (before == after)
        {
            return Ok();
        }

        // Emit 3046 self
        {
            std::vector<WireU8> frame;
            const WireU32 hpNow = session->state.GetCurrent(Resources::ResourceKind::Hp);
            const WireU32 hpMax = session->state.GetMaximum(Resources::ResourceKind::Hp);
            const WireU32 mpNow = session->state.GetCurrent(Resources::ResourceKind::Mp);
            const WireU32 mpMax = session->state.GetMaximum(Resources::ResourceKind::Mp);
            const WireU32 spNow = session->state.GetCurrent(Resources::ResourceKind::Sp);
            const WireU32 spMax = session->state.GetMaximum(Resources::ResourceKind::Sp);

            if (const Status st = Network::UpdateState::UpdateStateCodec::AppendStateUpdate(frame,
                {
                    { hpNow, hpMax },
                    { mpNow, mpMax },
                    { spNow, spMax },
                    { 0, 0 },
                    session->name,
                    session->gaeaId,
                    session->characterId,
                    false
                });
                st.IsOk())
            {
                SelfFrameSink selfCopy = session->selfSink;
                WriteBackSink wbCopy   = session->writeBackSink;
                selfCopy(frame);
                wbCopy({ hpNow, hpMax }, { mpNow, mpMax }, { spNow, spMax });
            }
        }

        // HP restore also emits 3053
        if (kind == Resources::ResourceKind::Hp)
        {
            std::vector<WireU8> frame;
            const WireU32 hpNow = session->state.GetCurrent(Resources::ResourceKind::Hp);
            const WireU32 hpMax = session->state.GetMaximum(Resources::ResourceKind::Hp);

            if (const Status st = Network::UpdateState::UpdateStateCodec::AppendStateBroadcast(frame,
                {
                    session->gaeaId,
                    { hpNow, hpMax },
                    false
                });
                st.IsOk())
            {
                HpFrameSink hpCopy = session->hpSink;
                hpCopy(frame);
            }
        }

        return Ok();
    }

    WireU32 ResourceSyncService::ApplyDamage(WireU32 gaeaId, WireU32 amount)
    {
        if (amount == 0)
        {
            return 0;
        }

        Session* session = nullptr;
        {
            std::lock_guard lock(m_mutex);
            auto it = m_sessions.find(gaeaId);
            if (it == m_sessions.end())
            {
                return 0;
            }
            session = &it->second;
        }

        const WireU32 before = session->state.GetCurrent(Resources::ResourceKind::Hp);
        if (before == 0)
        {
            return 0;
        }

        const WireU32 actual = session->state.ApplyDamage(amount);
        if (actual == 0)
        {
            return 0;
        }

        // Emit 3046 self
        {
            std::vector<WireU8> frame;
            const WireU32 hpNow = session->state.GetCurrent(Resources::ResourceKind::Hp);
            const WireU32 hpMax = session->state.GetMaximum(Resources::ResourceKind::Hp);
            const WireU32 mpNow = session->state.GetCurrent(Resources::ResourceKind::Mp);
            const WireU32 mpMax = session->state.GetMaximum(Resources::ResourceKind::Mp);
            const WireU32 spNow = session->state.GetCurrent(Resources::ResourceKind::Sp);
            const WireU32 spMax = session->state.GetMaximum(Resources::ResourceKind::Sp);

            if (const Status st = Network::UpdateState::UpdateStateCodec::AppendStateUpdate(frame,
                {
                    { hpNow, hpMax },
                    { mpNow, mpMax },
                    { spNow, spMax },
                    { 0, 0 },
                    session->name,
                    session->gaeaId,
                    session->characterId,
                    false
                });
                st.IsOk())
            {
                SelfFrameSink selfCopy = session->selfSink;
                WriteBackSink wbCopy   = session->writeBackSink;
                selfCopy(frame);
                wbCopy({ hpNow, hpMax }, { mpNow, mpMax }, { spNow, spMax });
            }
        }

        // Emit 3053 broadcast
        {
            std::vector<WireU8> frame;
            const WireU32 hpNow = session->state.GetCurrent(Resources::ResourceKind::Hp);
            const WireU32 hpMax = session->state.GetMaximum(Resources::ResourceKind::Hp);

            if (const Status st = Network::UpdateState::UpdateStateCodec::AppendStateBroadcast(frame,
                {
                    session->gaeaId,
                    { hpNow, hpMax },
                    false
                });
                st.IsOk())
            {
                HpFrameSink hpCopy = session->hpSink;
                hpCopy(frame);
            }
        }

        return actual;
    }

    const Resources::ResourceState* ResourceSyncService::Find(WireU32 gaeaId) const
    {
        std::lock_guard lock(m_mutex);
        const auto it = m_sessions.find(gaeaId);
        if (it == m_sessions.end())
        {
            return nullptr;
        }
        return &it->second.state;
    }

    std::size_t ResourceSyncService::SessionCount() const noexcept
    {
        std::lock_guard lock(m_mutex);
        return m_sessions.size();
    }
}