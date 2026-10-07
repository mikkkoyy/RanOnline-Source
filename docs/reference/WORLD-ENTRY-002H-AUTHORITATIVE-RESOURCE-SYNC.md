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

## Latent Race Fixed
**Pre-existing**: `SendEnveloped` compressed with per-peer `MinLzo1xCodec` OUTSIDE `peer->sendMutex` (BroadcastGoto from another worker thread races the peer's own worker).
**Fix**: Hold `sendMutex` across encode+send in `SendEnveloped` (restructured with private unlocked send helper; `SendRaw` removed to avoid double-lock deadlock).

## Testing

### Unit Tests (Headless)
- `modern/tests/UpdateStateProtocolTests.cpp` (ModernNetworkTests): layout static asserts, round-trip, boundaries, malformed, predicates
- `modern/server/ResourceSyncServiceTests.cpp` (ModernServerTests): register/unregister, recovery formula, timer (1.6s reset-to-zero, dead gate), Spend/Restore/ApplyDamage emissions, write-back, multi-session isolation, MP/SP floor 0, HP dead gate

### Client Gameplay Tests
- `modern/client/gameplay/ClientGameplayTests.cpp`: `ApplyResourceUpdate` changes presented pools; maxima untouched

### TCP Integration Tests
- `modern/server/WorldEntryTcpTests.cpp`:
  - Full flow → `server.Runtime().Field().ResourceSync().Advance(1.7f)` → 3046 received with spawn pools
  - Recovery advances pools
  - Two-client: self-only 3046
  - `ApplyDamage` → A gets 3046, B gets 3053
  - Fragmentation via existing `kFragmentSizes` machinery

### CMake Registration
- `modern/network/CMakeLists.txt`: `UpdateStateProtocol.cpp`
- `modern/server/CMakeLists.txt`: `ResourceSyncService.cpp` + `ResourceSyncServiceTests.cpp`
- `modern/tests/CMakeLists.txt`: `UpdateStateProtocolTests.cpp`

## Build & Verification
```cmd
cmd /c 'call "D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul && cmake --build build-debug --config Debug'
cmd /c 'call "D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul && cmake --build build-release --config Release'
cmd /c 'call "D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul && ctest --test-dir build-debug --output-on-failure'
cmd /c 'call "D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul && ctest --test-dir build-release --output-on-failure'
```

## Commit
```
WORLD-ENTRY-002H: implement authoritative resource sync (3046/3053 + 1.6s tick)
```