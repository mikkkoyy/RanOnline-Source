# WORLD-ENTRY-002H: Authoritative HP/MP/SP Resource Synchronisation

## Summary
Implements the authoritative resource synchronisation for HP, MP, SP (and dead-field CP) using the legacy `SNETPC_UPDATE_STATE` (3046) and `SNETPC_UPDATE_STATE_BRD` (3053) message family, with the 1.6-second recovery tick (`UPDATE_DATA` path) exactly as the legacy server does.

**Baseline**: `5f85865` (WORLD-ENTRY-002G, HEAD==origin/main, clean)

## Legacy Reference (Locked)

### Wire Protocol
- **3046** (`SNETPC_UPDATE_STATE`, 82 bytes, `#pragma pack(1)`)
  - Offsets (proven from source):
    - `nmg.dwSize`@0 = 82, `nmg.nType`@4 = 3046
    - `sHP.dwNow`@8, `sHP.dwMax`@12 (GLDWDATA, 8 bytes each)
    - `sMP`@16, `sSP`@24, `sCP`@32 (always {0,0})
    - `szCharName[33]`@40
    - `dwCharGaeaID`@73, `dwCharID`@77, `bSafeTime`@81
  - Source: `legacy/Lib_Client/G-Logic/GLContrlPcMsg.h:946-975`, `legacy/Lib_Network/s_NetGlobal.h:1013`

- **3053** (`SNETPC_UPDATE_STATE_BRD`, 21 bytes)
  - Derives `SNETPC_BROAD` (nmg@0, dwGaeaID@8), then `sHP`@12, `bSafeTime`@20
  - Source: `legacy/Lib_Client/G-Logic/GLContrlPcMsg.h:976-989`, `s_NetGlobal.h:1021`

### Legacy Behaviour
- `GLChar::MsgSendUpdateState` (GLCharMsg.cpp:37): event-driven self 3046 + 3053 to view-around
- `GLChar::UpdateClientState` timer (GLChar.cpp:5853): fires every **1.6s**, **reset-to-zero** (`m_fSTATE_TIMER=0`), gated on `m_sHP.dwNow > 0` (dead = no recovery, no timer)
- Recovery formula in `UPDATE_DATA` (GLogixExPC.cpp:2183, regen :3019-3026):
  ```
  fElap * (dwMax * fINCR + fX_INC + itemFlat)
  ```
  with fractional carry via `GLOGICEX::UPDATE_POINT`
- HP floors at 1 when alive (0 = death via `DoFalling` GLChar.cpp:7131), MP/SP floor at 0
- Global rates: `fHP_INC_PER=fMP_INC_PER=0.003`, `fSP_INC_PER=0.005`, flats = 0

## Modern Architecture

### Core Reuse
- `modern/core/resources/ResourceState.{h,cpp}`: complete with `ClampToMaximum`, `FullRestore`, `Spend`, `ApplyDamage`, `Restore`, `Recover` (fractional carry), `RespawnRestore`
- `modern/core/stats/StatCalculator.cpp:491-504`: uses `RecoveryRateConstant::kHp=0.003`, `kMp=0.003`, `kSp=0.005`, all flats 0.0f — exactly the legacy globals when no items/passives/facts exist
- `modern/server/world/WorldCharacter.h`: authoritative record with `hp/mp/sp` as `RanWire::DwPair{now,max}`
- `RanWire::DwPair` at `modern/network/RanWirePrimitives.h:94` (fields `now`/`max`)

### New Components

#### `modern/network/UpdateStateProtocol.h/.cpp`
Complete codec for 3046/3053:
- Message IDs, sizes, offsets as `static_assert`
- `StateUpdateWire` / `StateBroadcastWire` pack(1) structs
- App-layer `StateUpdate` / `StateBroadcast` with `DwPair` pools
- `UpdateStateCodec::{Append,Decode}{StateUpdate,StateBroadcast}`
- `IsStateUpdate` / `IsStateBroadcast` predicates
- `kNameFieldSize=33` cross-asserted against `WorldEntry::kNameFieldSize`

#### `modern/server/world/ResourceSyncService.{h,cpp}`
Per-session authority:
- `RegisterSession(WorldCharacter, SelfFrameSink, HpBroadcastSink, WriteBackSink)`
- `UnregisterSession(gaeaId)`
- `Advance(float elapsed)` — recovery + 1.6s reset-to-zero timer, dead-gate on hp==0
- Event seams: `Spend`/`Restore`/`ApplyDamage` (emit 3046 self on change, 3053 on HP change, write-back on emit)
- Single mutex; callbacks invoked outside the lock
- Recovery config: legacy globals via `StatCalculator::RecoveryRateConstant` (no items/passives/facts yet)

#### `modern/server/world/FieldRoleRuntime` Integration
- Member `m_resources` (`ResourceSyncService`)
- Register in `HandleIdentity` after `m_movementWorld.Attach` (FieldRoleRuntime.cpp:~496)
- Unregister in `ServePeer` teardown
- Resource ticker thread mirroring movement ticker's 4ms measured-elapsed pattern with 1.0s clamp
- `StartResourceTicker`/`StopResourceTicker` — idempotent stop called unconditionally at top of `Stop()` to avoid dangling thread on double-Stop
- `ResourceSync()` test accessor
- Atomic counters `UpdateStateSentCount`/`UpdateStateBrdSentCount`
- `BroadcastResourceState` peer fan-out excluding self

#### `modern/client/world/WorldEntryClient` + `FieldConnection`
- `WorldUpdateStateState` / `WorldUpdateStateBrdState` structs + accessors
- Handle 3046/3053 in `DrainFieldMessages` BEFORE spawn test (same as 3033/3035)
- `PumpUntilUpdateStateCount` / `PumpUntilUpdateStateBrdCount` (COUNT-based, mirrors `PumpUntilMoveCount`)

#### `modern/client/gameplay/ClientCharacterState`
- `ApplyResourceUpdate(hp, mp, sp)` — sets snapshot pool currents; maxima live in `derived` (server authority)

## Design Decisions

### 1. No Initial 3046 at Spawn
**Decision**: Legacy 2333 carries pools; first 3046 fires at first 1.6s timer or first event.
**Rationale**: Legacy `MsgSendUpdateState` is never called during spawn. Inventing a spawn-time 3046 would be a new message not in the protocol.

### 2. 3053 Broadcast Scope (Staged Approximation)
**Legacy**: Party/confront/hostile/PvP-map only (GLChar.cpp:5853 `MsgSendUpdateState(false)` = self-only; party/confront adds 3053 via `SendMsgViewAround`).
**Modern**: Broadcasts HP changes to **all authorized peers** (mirroring 3033/3035 precedent).
**Documentation**: This deviation is documented here and in the service header. Sector system arrives with the map milestone.

### 3. CP (Contribution Points) — Dead Wire Field
**Decision**: Always encoded as `{0,0}` in 3046. No producer exists in 002H.
**Rationale**: Legacy carries it; removing it would change the wire size. 3047/3048/3049/3050/3052 (EXP/MONEY/SP/LP/SKP) deferred — no producer yet.

### 4. Write-Back to Repository
**Decision**: Via `InMemoryCharacterRepository::Replace` at emit time only.
**Rationale**: The authoritative pools live in `WorldCharacter` in the repository. The service mutates its local `ResourceState` and writes back on every emit.

### 5. Recovery Config
**Decision**: Legacy global constants (exactly StatCalculator's empty-contribution result).
**Rationale**: No `default.charclass` loader exists yet; item/passive/fact terms arrive with equipment milestone.

## Defects Found and Fixed (stabilization pass)

The 002H implementation shipped with four defects that no test had been able to
see, and one of them was not a concurrency problem at all.

### 1. The client silently dropped every 3035 (the GOTO regression)

**This was the cause of the four failing GOTO TCP tests, and it had nothing to do
with the resource ticker.**

`WorldEntryClient::PumpField` dispatches every message that arrives on the Field
connection: 3033, 3035, 3053, 3046, and finally the 2333 spawn. Adding the 3053
and 3046 blocks REPLACED the 3035 block instead of being added alongside it. A
3035 therefore fell through to the "not a spawn" branch and was silently
discarded, so `m_gotoCount` stayed at zero and `Goto()` stayed unreceived.

The proof that this was a client-side dispatch fault and not a server or ticker
fault is a counter asymmetry in the existing boundary test:

| Counter | Value | Meaning |
|---|---|---|
| `FieldRoleRuntime::GotoSentCount()` | `1` (passes) | the server built and sent the 3035 |
| `client.connection.GotoCount()` | `0` (fails) | the client never counted it |

The server had already done everything correctly. The 3035 was on the wire and
was thrown away on arrival. **Fix**: the 3035 dispatch block is restored, ahead
of the spawn test, alongside the 3046/3053 blocks.

This is the reason the earlier hypothesis - that the resource ticker's thread
scheduling starved the GOTO worker - was wrong. The GOTO tests never start the
ticker, so no ticker thread existed during the failure, and the tests failed
identically with the ticker disabled.

### 2. `Pool` field order was transposed, destroying every character's maxima

`RanWire::DwPair` is `{ now, max }`. `Resources::Pool` is
`{ maximum, current }` - **opposite order**. `RegisterSession` built its
`ResourceRecord` with brace initialisation from the `DwPair`, so it compiled,
read as obvious, and put the CURRENT value in the MAXIMUM slot.

The consequences were not cosmetic:
- recovery accrued against the wounded value rather than the real maximum,
- `Restore` clamped at the wounded value, which makes **damage permanent** - a
  character that took 500 damage could never heal past that value,
- every 3046 told the client a maximum that was not the character's.

**Fix**: field-named assignment (`record.hp.maximum = character.hp.max;
record.hp.current = character.hp.now;`), with a comment recording why the
brace form is wrong here. Brace initialisation across the two types is now
impossible to reintroduce by accident at this call site.

### 3. Dangling `Session*` in the mutators

`Spend`, `Restore`, `ApplyDamage` and `Find` each looked a session up under
`m_mutex`, released the lock, and then kept reading and writing through the raw
pointer - while the peer's own worker thread could call `UnregisterSession` and
erase the map entry underneath them.

That is a use-after-free, and a `std::map` erase also rebalances the tree, so
the pointer could be invalidated without any write to the memory it pointed at.
**Fix**: the mutex is now held across the lookup, the mutation AND the frame
encoding, and everything the caller needs to say to the outside world is
captured in an `Outgoing` record (sinks copied, frames encoded once) before the
lock is released. `Deliver` then runs with no lock held and touches no `Session`
reference at all, which is the property that makes the pattern safe.

`Find` no longer returns `const ResourceState*` into the map. It returns
`std::optional<ResourceState>` - a copy - because a returned pointer's validity
would depend on which thread happened to call next. `Current(kind)` is the
convenient single-number form.

### 4. `Stop()` could leave the ticker thread joinable, which is `std::terminate`

`FieldRoleRuntime::Stop()` began with:

```cpp
if (!m_listener.IsListening() && !m_acceptThread.joinable())
{
    return;
}
```

A role whose resource ticker was started without a listening Field returned here
and never joined the thread. The destructor calls `Stop()`, and `Stop()` is the
only code that ever joins, so the `std::thread` destructor ran on a joinable
thread: `std::terminate`.

**Fix**: the running flag participates in the early-out, and `StopResourceTicker()`
runs at the TOP of `Stop()` - before the listener and before the peers - because
the ticker is the only thread not driven by `Stop()`'s own shutdown sequence, and
the only one that can enter the resource service while sessions are being torn
down.

### 5. Spawn ordering: registration and the `spawned` flag both trailed the 2333

Two orderings had the client's clock ahead of the server's:

- `RegisterSession` ran **after** the 2333 was sent. A client that had already
  read its spawn could get `NotFound` from `Spend`/`Restore`/`ApplyDamage` for
  its own character. Now the session is registered **before** the send, and a
  refusal is fatal to the spawn - a character with no authoritative pools cannot
  be repaired later, because registration happens exactly once.
- `peer->spawned` was raised **after** the send. It gates every broadcast (3033,
  3035, 3053), so a fully spawned and connected player was invisible to everyone
  else's broadcast for a window. The only symptom was a 3053 that never arrived
  for a character standing right there. The flag is now raised before the send.

Both were found by running the suite 20 times, not by reasoning: each is a
narrow window that a single run does not hit.

### 6. Unchecked `RegisterSession` status

The returned `Status` was discarded with `(void)`. A silent failure is
indistinguishable on the wire from "resource sync does not exist yet": the client
simply never receives a 3046. It is now checked, recorded in
`ResourceRegisterFailure()`, and fatal to the spawn.

## Ticker Lifecycle

| Property | How it is enforced | Test |
|---|---|---|
| Starts | `m_resourceTickerRunning.exchange(true)` | `ResourceTicker_AdvancesPoolsAndStopsOnStop` |
| A second Start is refused, not ignored | the same `exchange` returns `AlreadyExists`; two threads advancing one set of pools is a race with no correct outcome | `ResourceTicker_ARepeatedStartIsRefused` |
| Runs at a fixed slice rate | 4 ms sleep, mirroring the movement ticker; the old loop spun with no sleep at all | `ResourceTicker_AdvancesPoolsAndStopsOnStop` |
| Measures its own elapsed time | `steady_clock` delta between two real instants, clamped to 1.0 s | as above |
| A negative delta cannot reach a pool | clamped to 0 before `Advance` | as above |
| Stop waits for termination | `StopResourceTicker()` joins | `ResourceTicker_StopJoinsAndIsIdempotent` |
| Stop is idempotent | the `exchange(false)` returns false on a repeat and returns early | as above |
| Stop runs before peer teardown | `StopResourceTicker()` is the first statement in `Stop()` after the guard | `ResourceTicker_StopJoinsAndIsIdempotent` (the destructor at end of scope is part of the assertion) |
| Unregister cannot race advancement | every mutator holds `m_mutex` across lookup AND mutation; `UnregisterSession` erases under the same lock | `ResourceSync_UnregisterCannotRaceWithAdvancement` (300 iterations, two threads) |
| A GOTO and a recovery update coexist | 3035 from the peer's worker thread, 3046 from the ticker thread, one connection | `ResourceSync_ARecoveryUpdateAndAGotoCoexistOnOneConnection` |

Every wait in these tests is a `WaitFor` on an observable property. No `Sleep()`
was added to make a race disappear.

## Testing Notes

`ResourceSyncServiceTests.cpp` had been excluded from `ModernServerTests` with the
note "temporarily disabled - test file needs fixes". It is now built and passing;
restoring it is what exposed defects 2 and 3.

The file also needed three genuine fixes, not just mechanical ones:

- `REQUIRE` did not exist. The harness had `CHECK` only, so `CHECK(state !=
  nullptr); state->GetCurrent(...)` flagged a failure and then dereferenced null
  anyway - which crashed the whole executable with `0xc0000409` instead of
  reporting the one broken assertion. `REQUIRE` was added to `TestHarness.h`.
- The fixture produced characters that `WorldCharacter::Validate` refuses (zero
  account id, empty userId, level 0), so every `repo.Add` silently failed and
  the repository was empty. The write-back test could never have passed.
- The write-back test's sink only RECORDED the pools. The service never touches
  the repository itself - that is the write-back sink's job, which is why
  `FieldRoleRuntime::WriteBackPools` exists. A sink that does not call
  `Replace` leaves the repository untouched, so the test asserted something no
  code was responsible for.

One expectation was wrong rather than the code, and it is worth recording:
`ResourceSyncService_Recovery_SpRecovers_WithLegacyRate` expected 103 SP after
one tick. The actual answer is 102. The legacy rate is `0.5f * 0.01f`, and
`0.005f` is 0.004999999888241291 in IEEE single precision - 2.99999993 points
over a 600-point maximum. `UPDATE_POINT` truncates toward zero, so the tick pays
two whole points and carries the fraction. A second tick collects it and pays
105 in total, which is within one point of `2 * 2.99999...`. Rounding the rate up
to reach 103 would make the server disagree with legacy in the client's favour.

## Build & Verification

```cmd
cmake --build build-debug --config Debug
cmake --build build-release --config Release
ctest --test-dir build-debug -C Debug
ctest --test-dir build-release -C Release
```

Results:

| Check | Debug | Release |
|---|---|---|
| Build | 0 errors, 0 warnings | 0 errors, 0 warnings |
| `ModernServerTests` | 293 / 293 | 293 / 293 |
| `ModernWorldEntryTcpTests` | 29 / 29 | 29 / 29 |
| Full CTest | 18 / 18 | 18 / 18 |
| `ModernServerTests` x20 | 0 failures | 0 failures |
| `ModernWorldEntryTcpTests` x20 | 0 failures | 0 failures |

## Commit
```
WORLD-ENTRY-002H: fix resource and GOTO concurrency lifecycle
```
