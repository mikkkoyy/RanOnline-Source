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
#include <optional>
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

        // Returns a COPY of the resource state for `gaeaId`, or nullopt if not
        // tracked.
        //
        // A copy, and not a pointer into m_sessions, because the map is erased
        // by UnregisterSession on the peer's own worker thread. A returned
        // pointer would be a dangling reference the instant a client
        // disconnects, and the lifetime of the thing it pointed at would
        // depend on which thread happened to call next. Copying one small
        // record under the mutex is not a cost worth that ambiguity.
        std::optional<Resources::ResourceState> Find(Network::WireU32 gaeaId) const;

        // Pool value for `gaeaId`, or `fallback` when `gaeaId` is not tracked.
        // The convenient, and equally safe, form of Find for a single number.
        Network::WireU32 Current(Network::WireU32 gaeaId,
                                 Resources::ResourceKind kind,
                                 Network::WireU32 fallback = 0) const;

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

        // Everything one mutation wants to say to the outside world, captured
        // while the session is known to exist.
        //
        // The sinks may write to a socket, and a sink that blocks while
        // m_mutex is held would stop the ticker, every other session's
        // recovery, and the peer's own disconnect path. So the mutation runs
        // under the lock and the CALLBACKS are copied into one of these; the
        // frame is encoded once, under the lock, rather than re-encoded later
        // against a session that may no longer be there.
        struct Outgoing
        {
            bool                hasSelf = false;
            SelfFrameSink       self;
            std::vector<Network::WireU8> selfFrame;
            WriteBackSink       writeBack;
            Network::RanWire::DwPair hp{};
            Network::RanWire::DwPair mp{};
            Network::RanWire::DwPair sp{};

            bool                hasHp = false;
            HpFrameSink         hpSink;
            std::vector<Network::WireU8> hpFrame;
        };

        // Encodes the 3046 for `session` and records it in `out`. Mutates
        // nothing. Requires m_mutex held.
        static void BuildSelfFrame(const Session& session, Outgoing& out);

        // Encodes the 3053 for `session` and records it in `out`. Requires
        // m_mutex held.
        static void BuildHpFrame(const Session& session, Outgoing& out);

        // Invokes the recorded callbacks. Takes NO lock, and takes no
        // reference to any Session - which is the entire point of Outgoing.
        static void Deliver(const Outgoing& out);

        mutable std::mutex m_mutex;
        std::map<Network::WireU32, Session> m_sessions;
    };
}