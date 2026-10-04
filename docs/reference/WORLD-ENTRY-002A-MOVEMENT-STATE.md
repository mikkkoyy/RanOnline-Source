# WORLD-ENTRY-002a: movement state

Implements the RAN movement-STATE protocol - `NET_MSG_GCTRL_MOVESTATE` (3032) and
`NET_MSG_GCTRL_MOVESTATE_BRD` (3033) - over the Field connection built by
WORLD-ENTRY-001.

Scope is decided by the investigation in
[WORLD-ENTRY-002-MOVEMENT-INVESTIGATION.md](WORLD-ENTRY-002-MOVEMENT-INVESTIGATION.md):
this milestone is the *state half only*. Nothing here moves a character anywhere.

## What was built

| Layer | File | What it owns |
|---|---|---|
| Wire | `modern/network/MovementStateProtocol.{h,cpp}` | 3032 (12 bytes) and 3033 (16 bytes), measured layouts, flags |
| Rule | `modern/server/world/MovementStateService.{h,cpp}` | which bits a client may influence, as a pure function over `WorldCharacter` |
| Role | `modern/server/world/FieldRoleRuntime.{h,cpp}` | 2359 → 2333 → 3032 → 3033, on a long-lived connection |
| Client | `modern/client/world/WorldEntryClient.{h,cpp}` | `BuildMoveState`, 3033 decoding |
| Client | `modern/client/world/WorldEntryConnections.{h,cpp}` | `SendMoveState`, `PumpUntilMoveCount` |

## The three rules that matter

Each is reproduced from `GLChar::MsgMoveState` (`GLCharMsg.cpp:182-219`) and each is
a security property rather than a formatting detail.

**1. The server never stores the client's value.** Legacy applies *individual bits*
to a persistent word. Bits the client does not own - `EM_ACT_DIE`, `EM_REQ_GATEOUT`,
`EM_REQ_LOGOUT`, `EM_ACT_WAITING`, `EM_ACT_CONFT_WIN`, `EM_ACT_PK_MODE` - are
*preserved*. A client cannot become "dead" or "log me out" by putting the bit in a
3032, because the server never reads it.

**2. Below `USER_GM3` the two visibility flags are left alone** - neither set nor
cleared. `EM_REQ_VISIBLENONE` and `EM_REQ_VISIBLEOFF` sit inside the account-level
gate, so an ordinary client can neither raise nor lower them. The gate is on
*account* level (`accountLevel`), never character level; conflating the two would
hand GM visibility to anyone who levelled.

**3. An unchanged word produces no broadcast at all.** Legacy compares the whole
32-bit word before and after (`GLCharMsg.cpp:186,203`) and sends nothing when it
matched. That silence is observable behaviour and is asserted as an absence in
`MovementState_AnUnchangedMoveProducesNoBroadcastAtAll`.

The 3033 carries the **whole authoritative word**, not the client's delta, so a
client cannot have to guess the server-owned bits.

## The Field connection became long-lived

This is the structural change, and it is forced by the protocol rather than chosen.
A 3032 arrives minutes after the 2359, on a socket that must still be there.

`FieldRoleRuntime` therefore grew an accept thread and one worker thread per
connection. `ServeOneFieldClient` is gone: a one-shot entry point would have served
one client to completion and deadlocked the moment a second client was watching the
first move.

Two counters changed meaning, and it is worth being explicit:

- `ServedClientCount` is incremented **at the spawn**, not at end of connection.
  Before 002a one connection was one conversation and the number meant "served a
  conversation". Now that a connection outlives the conversation, counting at the
  end would make the answer depend on how the client hung up.
- `RefusedClientCount` counts connections that ended because the client broke a
  rule. An orderly close and an idle timeout are in **neither** counter. Letting
  them count as refusals would drown the refusals a caller actually needs to see.

The per-message read budget (10s) replaced the whole-conversation budget, which is
what allows a client to sit idle between moves.

## Three concurrency defects this exposed

Making the Field role multi-threaded made three latent bugs reachable. All three
were found by a 20-run sweep of the TCP tests, not by inspection, and all three are
production bugs rather than test problems.

1. **A shared `MinLzo1xCodec`.** `CompressionCodec.h` states outright that instances
   are "NOT internally synchronised; a server with concurrent senders should hold
   one per thread". The role had one member codec shared by every worker; the LZO
   work buffer is now per-connection, beside the transport it serves. Symptom was a
   corrupt envelope surfacing as `InvalidArgument` on an innocent client's socket.

2. **An unsynchronised `InMemoryCharacterRepository`.** Two workers reading while
   the Agent thread wrote. `std::map` rebalancing observed half-done is a real
   failure, not a theoretical one.

3. **A non-atomic check-then-consume in `FieldEntryRegistry::Claim`.** Two clients
   racing to claim one gaeaId could both pass the replay check and both be told they
   had entered the world, with two characters sharing one entity id. The lock is now
   held across `Validate` *and* the consume, which is why it is recursive.

Lock order is registry-then-repository everywhere; `Reserve` takes the registry lock
only after its repository calls have returned, so the reverse order never nests.

## The speed seam has no implementation, deliberately

Legacy recomputes `GetMoveVelo()` on every change, which reads `cCONSTCLASS` from
`default.charclass` - not in this repository - plus worn equipment, which
WORLD-ENTRY-001 excludes. So `IMovementSpeedProvider` is declared and **no
implementation is supplied**. With no provider the change still applies and the
broadcast still goes out; `speedUnavailable` records why the speed is absent, rather
than a silent `0.0` that would be indistinguishable from a genuinely slow character.

## Broadcast scope is a staged approximation

A changed state is sent to every currently authorized Field session, minus the
mover (which receives its own copy synchronously, on the connection it moved from).
Legacy uses `SendMsgViewAround` - sector-scoped visibility - and that infrastructure
is not being reconstructed here. The approximation is documented rather than hidden.

## Deliberately not in this milestone

`GOTO` (3034/3035), position movement, NaviMesh, interpolation, and full sector
visibility. GOTO needs ~5374 lines of NaviMesh, map data that is absent, and the
sector/view machinery; it is WORLD-ENTRY-002b.

## Tests

- `MovementStateServiceTests` - 16 headless cases: application, authority,
  privilege, change detection, the speed seam, and byte-level encoding.
- `WorldEntryTcpTests` - 7 cases over real sockets: own-3033, cross-client
  broadcast, no-op silence, server-owned bits over the wire, pre-entry refusal,
  malformed 3032, and survival across an idle gap.

Suite: `ModernNetworkTests 193/193`, `ModernServerTests 228/228`,
`ModernWorldEntryTcpTests 14/14`, CTest Debug and Release `18/18`.