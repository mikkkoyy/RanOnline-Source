#pragma once

#include "UpdateStateProtocol.h"
#include "resources/ResourceState.h"
#include "stats/StatCalculator.h"
#include "world/WorldCharacter.h"
#include "types/Result.h"
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace Modern::Server::World
{
    class ResourceSyncService
    {
    public:
        using SelfFrameSink = std::function<void(const std::vector<Network::WireU8>&)>;
        using HpFrameSink = std::function<void(const std::vector<Network::WireU8>&)>;
        using WriteBackSink = std::function<void(const Network::RanWire::DwPair&,
                                                  const Network::RanWire::DwPair&,
                                                  const Network::RanWire::DwPair&)>;

        // Registers a new authoritative session for a character. The
        // character's hp/mp/sp DwPairs are the source of truth for both
        // current and maximum values. The sinks are invoked *outside* the
        // service mutex when a frame is ready to send or write-back.
        //
        // Refused if gaeaId is 0 (not in the world) or already registered.
        Status RegisterSession(const WorldCharacter& character,
                                SelfFrameSink self,
                                HpFrameSink broadcast,
                                WriteBackSink writeBack);

        // Removes the session. No frame is emitted.
        Status UnregisterSession(Network::WireU32 gaeaId);

        // Advances the recovery simulation and the 1.6s state timer.
        // `elapsedSeconds` must be positive and finite; anything else is
        // ignored (no recovery, no timer advance). This matches the legacy
        // behaviour where `UPDATE_DATA` is driven by the frame elapsed time
        // and the timer is advanced only for living characters.
        void Advance(float elapsedSeconds);

        // Spends from a pool. If the pool changed, emits a 3046 self frame.
        // HP spend also emits a 3053 broadcast. Returns the amount actually
        // spent (saturating at current). A non-positive `amount` is a no-op.
        Status Spend(Network::WireU32 gaeaId,
                      Resources::ResourceKind kind,
                      Network::WireU32 amount,
                      Network::WireU32& spent);

        // Restores a pool up to its maximum. Emits 3046 if changed; HP
        // restore also emits 3053. A non-positive `amount` is a no-op.
        Status Restore(Network::WireU32 gaeaId,
                        Resources::ResourceKind kind,
                        Network::WireU32 amount);

        // Applies damage to HP (saturating at 0). Returns the amount
        // actually applied. Emits 3046 and 3053 if HP changed.
        Network::WireU32 ApplyDamage(Network::WireU32 gaeaId,
                                      Network::WireU32 amount);

        // Returns the resource state for `gaeaId`, or nullptr if not found.
        const Resources::ResourceState* Find(Network::WireU32 gaeaId) const;

        // Number of registered sessions.
        std::size_t SessionCount() const noexcept;

        // Legacy global constants:
        //   fHP_INC_PER = 0.003  (GLogixExPC.cpp:3021)
        //   fMP_INC_PER = 0.003
        //   fSP_INC_PER = 0.005
        //   all flat terms = 0
        // The StatCalculator mirrors these as RecoveryRateConstant::kHp/kMp/kSp
        // and RecoveryFlatConstant all zero.
        static constexpr float kStateTimerSeconds = 1.6f;

    private:
        struct Session
        {
            Network::WireU32             gaeaId;
            Network::WireU32             characterId;
            std::string                  name;
            Resources::ResourceState     state;
            float                        stateTimer = 0.0f;
            SelfFrameSink                selfSink;
            HpFrameSink                  hpSink;
            WriteBackSink                writeBackSink;
        };

        // Emits a 3046 self frame for the session. Called with m_mutex held
        // for mutation, but the sink is copied and invoked after unlock.
        void EmitSelfFrame(Session& session);

        // Emits a 3053 broadcast frame for the session. Called with m_mutex
        // held; sink copied and invoked after unlock.
        void EmitHpBroadcast(Session& session);

        // Invokes the write-back sink with the session's current pools.
        void InvokeWriteBack(Session& session);

        mutable std::mutex m_mutex;
        std::map<Network::WireU32, Session> m_sessions;
    };
}