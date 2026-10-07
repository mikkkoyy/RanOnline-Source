# WORLD-ENTRY-002G: next gameplay boundary investigation

Investigation only. No production gameplay subsystem was added. Baseline verified at
`19c8a6c6299809a96b66531f569e0b8ba835b617` (18/18 tests pass in `build-debug` and
`build-release`, 0 failed).

## 1. Executive result

```text
READY FOR IMPLEMENTATION
```

The next correct, smallest, bounded gameplay boundary after authoritative GOTO
movement (002F) is **authoritative HP/MP/SP resource synchronization over the Field
connection** — the `NET_MSG_GCTRL_UPDATE_STATE` family (`3046`, `UPDATE_STATE_BRD
3053`, `UPDATE_SP 3049`, `UPDATE_LP 3050`, `UPDATE_SKP 3052`, `UPDATE_MONEY 3048`),
including wiring the already-implemented recovery tick into the running Field role.

This is proven, not guessed:

- **Legacy ownership is proven.** Authoritative pools live in `SCHARDATA`
  (`m_sHP/m_sMP/m_sSP`, `legacy/Lib_Client/G-Logic/GLCharData.h:638-640`),
  maxima derive from `SUM_ADDITION` (`GLogixExPC.cpp:286`,
  `UPDATE_MAX_POINT GLogixExPC.cpp:2159`), regeneration runs in
  `UPDATE_DATA` (`GLogixExPC.cpp:2183`, called from `GLChar::FrameMove`
  `GLChar.cpp:5864`), damage writes `RECEIVE_DAMAGE` (`GLogixExPC.cpp:2093`),
  skills debit via `ACCOUNTSKILL` (`GLogixExPC.cpp:4349-4350`), skill affordability
  via `CHECHSKILL` (`GLogixExPC.cpp:4240-4261`), and the server mirrors all of it to
  clients event-driven through `GLChar::MsgSendUpdateState` (`GLCharMsg.cpp:37`,
  self `SNETPC_UPDATE_STATE`, `_BRD` to view-around).
- **Protocol gap is proven.** `modern/network` defines login, game-server-list,
  character-list, world-entry (2333/2353/2358/2359), MOVESTATE (3032/3033) and GOTO
  (3034/3035) only. No `UPDATE_*` (3046-3059), no ATTACK (3036), no REQ_SKILL (3303),
  no gate/land-in messages (3022-3025), no FIELDSVR_OUT (3007/3008) exist anywhere in
  `modern/`.
- **Modern dependency is ready.** `WorldCharacter` already carries hp/mp/sp (DwPair)
  on the authoritative session; `ResourceState` implements legacy saturation/recovery
  semantics and is saturated-used inside `ServerCharacter`, but its recovery tick is
  not driven by the world server; `DerivedStats` supplies `maxHp/maxMp/maxSp`;
  `ClientCharacterState` already presents pools client-side, off-wire. The only
  missing pieces are the wire codecs and the recovery/sync wiring.
- **Deterministic dependency.** Combat's damage outcome is *unobservable* without
  this sync: legacy `DamageProc` (`GLChar.cpp:2484`) → `ReceiveDamage` →
  `MsgSendUpdateState`. Death (`EM_ACT_DIE`, `DoFalling GLChar.cpp:7131`), revive
  (`MSG REVIVE 3466`), cure (`CURE 3397`), and rebirth all sync through the same
  family. Implementing ATTACK before 3046 would produce damage that never reaches the
  client — an architectural dead end. Implementing 3046 first does not foreclose any
  other ordering.
- **No inventory/equipment prerequisite.** Movement speed modifiers that DO require
  equipment (EMVAR_MOVE_SPEED, vehicle, disguise, passive movevelo, m_fOPTION_MOVE)
  are blocked because `item.csv` is not decoded into any modern `ItemDefinition`
  (only `reference/legacy-calculation-port/ItemData.h:393-397` models the fields).
  Resource sync does not need it.
- **Testability.** Headless: `ResourceState` recovery/spend/damage against injected
  elapsed time (same seam as `WorldMovementRuntime::Tick`). TCP: a real
  `WorldServerRuntime` session asserting the 3046/3033/3035 byte stream after a
  pool-changing event and after a recovery window, following the
  `WorldEntryTcpTests` pattern.

## 2. Current dependency graph

```text
LOGIN + game-server-list            (done, ModernLoginServer)
  └─ Agent role (2049/2050, 2247/2248, 2244/2332, 2353/2358)   (done, tests)
       └─ Field role (2359 → 2333)                              (done, tests)
            └─ MOVESTATE 3032/3033  movement state broadcast    (done, 002A)
            └─ GOTO 3034/3035       authoritative movement      (done, 002F)
                 └─ ┌─────────────────────────────────────────┐
                    │   DECISION POINT (002G investigation)   │
                    └─────────────────────────────────────────┘
                          │
   ┌──────────────────────┼──────────────────────────────┬────────────────────────┐
   ▼                      ▼                              ▼                        ▼
UPDATE_STATE 3046   ATTACK 3036                    Equipment/            Map transition
HP/MP/SP sync       (needs 3046 first,             inventory              (needs new Field
   │                needs target session,           (needs modern         connection + gate
   │                skills offline: needs item.csv  item.csv decode,      table decode;
   │                damage decode — BLOCKED         modern inventory      2359 EMJOINTYPE_
   │                today)                        model ABSENT)           MOVEMAP + 2358)
   ▼                      ▲
death / revive / cure /   │ damage outcomes ride MsgSendUpdateState
death-detect / rebirth────┘
   │
   ▼
ATTACK 3036 + ATTACK_DAMAGE 3043 + avoid → VERTICAL-016..027-resolved damage variants
   │
   ▼
REQ_SKILL 3303 (needs SP/MP cost sync = same UPDATE_STATE family + skill tables)
```

Branches not chosen as next: equipment/speed modifiers (blocked on item.csv decode),
direct combat (needs 3046 + ServerCharacter bridge), map transition (needs new-field
handshake support and gate data), another movement rewrite (002F locked).

## 3. Candidate milestone table

| Candidate | Legacy owner | Protocol readiness | Data readiness | Modern dependency readiness | Blockers | Risk | Recommendation |
|---|---|---|---|---|---|---|---|
| UPDATE_STATE 3046 family + recovery tick | `GLChar::MsgSendUpdateState` (GLCharMsg.cpp:37), `UPDATE_DATA` (GLogixExPC.cpp:2183) | Struct layouts known (GLContrlPcMsg.h:946/976/1018/1040/1055/1070); no modern codec | `ResourceState` implemented; `DerivedStats` maxima ready; hp/mp/sp on WorldCharacter | ServerCharacter.Resource path proven in tests; ticker seam exists (`WorldMovementRuntime::Tick`) | None real | Low; event-driven sync semantics must mirror legacy (no per-frame position-style broadcast) | **CHOOSE — WORLD-ENTRY-002H input** |
| ATTACK 3036 combat | `GLChar::MsgAttack` (GLCharMsg.cpp:326) | SNETPC_ATTACK layout known (GLContrlPcMsg.h:757); ATTACK_BRD/AVOID/DAMAGE known | Damage formula: VERTICAL-006..026 docs; but modern `ItemDefinition` lacks item.csv decode → base damage from items unproven | ServerCharacter.Attack exists but is test-only (never referenced by any server role) | item.csv decode; target/position session seam; death sync (needs 3046) | Medium | After 3046; first combat must be scoped (single target, distance from actor position, no item-derived range) |
| HP/MP/SP (without wire sync) | `GLCharData.h:638` | — | — | ResourceState complete | no network | Low | Subsumed by UPDATE_STATE milestone |
| Equipment + EMVAR_MOVE_SPEED | `GLogixExPC.cpp:562`, `GLItem.cpp:2746` | no inventory protocol defined | item.csv NOT decoded into modern ItemDefinition | no inventory model | item.csv modern decoder; modern inventory model | High (decode risk) | Blocked — do not start until item.csv decoder exists |
| Vehicle/disguise/passive/ramp speed terms | `GLVEHICLE.cpp:341`, `GLogixExPC.cpp:689/270` | — | vehicle data unproven | no vehicle/disguise state | above + passives pipeline | High | Blocked |
| Map transition 3022-3025/3007/3008 | `RequestGateOutReq` (GLGaeaServerMsg.cpp:895), `MsgReqGateOut` (GLAgentServerMsg.cpp:985) | structs known; modern has no codec; transition needs NEW Field TCP + new 2359 (EMJOINTYPE_MOVEMAP) | gate positions live in .lev/GaeaServer gate data — not proven decoded modern-side | FieldRoleRuntime binds a fixed map/registry; per-map runtime spawning unproven | gate table decode; Agent needs per-map Field endpoints | Medium | Valid later; not next (needs investigation first) |
| Client prediction beyond GOTO | — | — | — | WorldEntryClient complete for 3032/3034/3035 | none needed for 3046 (client already renders pools off-wire) | Low | Not standalone |

## 4. Proven packet inventory

| ID | Size (bytes) | Direction | Purpose | Source of truth |
|---|---|---|---|---|
| 3046 | sizeof(SNETPC_UPDATE_STATE): sHP(8)+sMP(8)+sSP(8)+sCP(8)+name+gaeaId+charId+safeTime — see note | server→client (self) | sync full pools + identity | GLContrlPcMsg.h:946, s_NetGlobal.h:1013 |
| 3053 | sizeof(SNETPC_UPDATE_STATE_BRD): sHP(8)+safeTime | server→view-around | pool broadcast to others | GLContrlPcMsg.h:976, s_NetGlobal.h:1021 |
| 3047 | sizeof(SNETPC_UPDATE_EXP) | server→client | exp | s_NetGlobal.h:1014 |
| 3048 | sizeof(SNETPC_UPDATE_MONEY) | server→client | money | s_NetGlobal.h:1015 |
| 3049 | 2 (wNowSP) + header | server→client | SP now | GLContrlPcMsg.h:1040, s_NetGlobal.h:1017 |
| 3050 | 4 (nLP) | server→client | LP | GLContrlPcMsg.h:1055 |
| 3052 | 4 (dwSkillPoint) | server→client | skill points | GLContrlPcMsg.h:1070 |
| 3058 | — | server→client | stats | s_NetGlobal.h:1018 |
| 3059 | — | server→client | flags | s_NetGlobal.h:1019 |
| 3036 | 12+ EMCROW/dwTarID/dwAniSel/dwFlags | client→server | attack request | GLContrlPcMsg.h:757, s_NetGlobal.h:1003 |
| 3037 | — | server→view | attack broadcast | s_NetGlobal.h:1004 |
| 3038/3039 | — | client→server / server→view | attack cancel | s_NetGlobal.h:1005 |
| 3041/3042 | — | server→client/view | avoid | s_NetGlobal.h:1007 |
| 3043/3044 | emTarCrow+dwTarID+nDamage+flag | server→client/view | damage outcome | GLContrlPcMsg.h:853 |
| 3303 | — | client→server | skill request | s_NetGlobal.h:1111 |
| 3022/3023/3024/3025 | — | client→field/field→agent/agent→field/client→field | gate-out handshake | GLContrlCharJoinMsg.h:723-773 |
| 3007/3008 | — | server→client/field | field-server-out | GLContrlCharJoinMsg.h:671 |
| 2358 | 24-ish | agent→client | connect-to-field (new TCP + new 2359 EMJOINTYPE_MOVEMAP) | s_NetGlobal.h:4281 |
| 2359 | carries emType/dwGaeaID/slot/crypt key | client→new field | field identity (no map/pos — carried server-side) | s_NetGlobal.h:4301 |
| 3466 | — | client→server | revive | s_NetGlobal.h:1312 |
| 3397 | — | client→server | cure | s_NetGlobal.h:1222 |

Sizes must be confirmed against `SNETPC_*` struct definitions (`nmg` header of 8
bytes: nType/nFlag/nSize-style prefix per `NetworkTypes`) before any codec lands;
the sizes above were *not* measured with a wire capture this milestone, so the
implementation milestone must `static_assert(sizeof(...))` each struct exactly as
002F did for 3034/3035.

## 5. Data inventory

| Data source | Format | Location | Decoded? | Modern reader | Runtime seam |
|---|---|---|---|---|---|
| Character current HP/MP/SP | `GLDWDATA dwNow/dwMax` (8B each) | DB columns ChaHP/ChaMP/ChaSP, s_COdbcGameChaGet.cpp:199 | n/a (runtime) | `WorldCharacter.hp/mp/sp` (`WorldCharacter.h`), DwPair | load on 2359 |
| Max HP/MP/SP | derived from Str/Spi/Sta + item/passive/codex sums | `SUM_ADDITION GLogixExPC.cpp:342-355` | yes, ported | `DerivedStats` in `modern/core/stats` | StatCalculator |
| Recovery rates | per-class constants + item flats | `UPDATE_DATA GLogixExPC.cpp:3019-3026` | yes, ported | `ResourceState::PoolRecovery` | ticker to wire |
| item.csv (`sVARIATE`/`sVOLUME` incl. EMVAR_MOVE_SPEED, damage) | CSV rows + options bins | `D:\FILES\project\RanOnline-Build\ASURA CLIENT\data\glogic\item.csv` | **NOT decoded** | only `reference/legacy-calculation-port/ItemData.h` models | needs modern decoder |
| mapslist.mst / .lev / .wld | binary, decrypted | ASURA client data | yes | `MapRegistry` (002E) | done |
| skill definitions / costs / cooldowns | editor tables | legacy EditorSkill / data files | partial (modern SkillDefinition exists, providers in-memory) | `SkillDefinitionProvider` | needs table import |
| gate positions / map transition targets | .lev gate data / GaeaServer gate tables | legacy server/GaeaServer | unproven modern-side | none | blocked until proven |

## 6. Explicit blockers

1. `item.csv` (and options bins) are not decoded into any modern `ItemDefinition`.
   Blocks equipment-speed modifiers, item-derived attack range/damage, vehicle and
   disguise speed, and any item-definition-accurate combat. NOT a blocker for the
   UPDATE_STATE milestone.
2. No modern inventory/equipment authoritative model (WorldCharacter has no
   equipment fields; `ItemInstance` defers per-copy options). Blocks all
   equipment-dependent milestones.
3. Combat is unobservable without 3046: `ATTACK_DAMAGE 3043` handlers write HP, and
   HP is mirrored via MsgSendUpdateState. Blocks ATTACK as the *next* milestone.
4. Map transition requires a new Field TCP + new 2359 with `EMJOINTYPE_MOVEMAP`,
   plus gate data modern readers do not yet have. Blocks map transition as the next
   milestone.
5. `ServerCharacter` is complete but never constructed by any server role (only
   tests). A "networked gameplay bridge" is needed before any combat/skills milestone
   can be real.

## 7. Recommended next milestone

```text
WORLD-ENTRY-002H — authoritative resource synchronization (UPDATE_STATE family)
```

not 002G combat, not equipment, not map transition. Rationale: strict protocol
prerequisite for every damage/cost/death outcome (legacy `DamageProc →
MsgSendUpdateState`), all modern state and math it needs already exist and are
tested (`ResourceState`, `DerivedStats`, `WorldCharacter` pools,
`ClientCharacterState`), it adds no inventory dependency, it has a deterministic
test story (headless recovery tick + real TCP byte assertions), and it does not
constrain any later milestone. The deferred 002F speed modifiers remain blocked on
the item.csv decoder; they do not block 002H.

## 8. Exact implementation boundary (for 002H)

```text
IN SCOPE
- modern/network: SNETPC_UPDATE_STATE (3046), SNETPC_UPDATE_STATE_BRD (3053),
  SNETPC_UPDATE_SP (3049), SNETPC_UPDATE_LP (3050), SNETPC_UPDATE_MONEY (3048),
  SNETPC_UPDATE_SKP (3052) codecs, static_asserted sizes, sharing the existing
  NetworkTypes framing precedent of 3032/3034.
- Field role: after a pool-changing event (spend/damage/restore — initially from
  tests since no damage source exists yet, and from skill/cost hooks later), emit
  3046; broadcast 3053 to view-around peers per legacy MsgSendUpdateState.
- Drive ResourceState recovery from the Field role's existing ticker using the same
  injected/real elapsed seam as WorldMovementRuntime; emit 3046 only on visible
  change (mirror legacy event-driven sends, not a per-frame broadcast).
- Client: WorldEntryClient decodes 3046/3053 -> ClientCharacterState pools.
- Tests: headless ResourceState + TCP test asserting 3046/3033/3035 ordering and
  exact byte layout; WaitFor-based peer assertions (no new races).

OUT OF SCOPE
- ATTACK/3036, REQ_SKILL/3303, damage formulas at runtime, death/revive/cure handlers,
  equipment/inventory, EMVAR_MOVE_SPEED, vehicle/disguise/passive/ramp speed terms,
  map transition (3022-3025/3007-3008/2358-2359-MOVEMAP), any per-tick position
  broadcast, any invented rejection packet.
```

## 9. Simple test plan

```bat
cmd /c "call \"%VS%\Common7\Tools\VsDevCmd.bat\" -arch=x64 >nul && ^
ctest --test-dir build-debug --output-on-failure -C Debug"
cmd /c "call \"%VS%\Common7\Tools\VsDevCmd.bat\" -arch=x64 >nul && ^
ctest --test-dir build-release --output-on-failure -C Release"
```

Expected today: 18/18 in both. For 002H itself (when implemented): new TCP test in
`ModernWorldEntryTcpTests` asserting a 3046 byte prefix and pool values after a
recovery window or spend; headless `ResourceState` recovery test with injected
elapsed time. No weakening of existing 18 tests.
