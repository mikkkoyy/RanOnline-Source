# WORLD-ENTRY-002f: authoritative GOTO movement

## What this milestone is

A client asks to walk somewhere (`3034`, `NET_MSG_GCTRL_GOTO`) and the Field role
decides whether it may, moves the character there itself, and answers with one
`3035` (`NET_MSG_GCTRL_GOTO_BRD`) carrying the position **the server** believes the
character is at. This is the first milestone where the server owns a character's
position rather than merely reporting bits about it.

It builds on `002d` (the navigation kernel), `002e` (`MapRegistry`) and `002a`
(`MovementStateService`, the shared speed seam).

## The two messages

| | id | size | layout |
|---|---|---|---|
| request | `3034` | 36 | `dwActState@8`, `vCurPos@12`, `vTarPos@24` |
| broadcast | `3035` | 44 | `dwGaeaID@8`, `dwActState@12`, `vCurPos@16`, `vTarPos@28`, `fDelay@40` |

The request carries **no `GaeaID`**. That is not an omission to be tidied up later: it
is why a client cannot move anybody but itself. The server fills the id in the answer
from its own authorized session.

Both codecs live in `modern/network/GotoProtocol.h` and are shared by the server and
the client, so "the client cannot emit a frame the server would refuse" is structural
rather than a promise.

## The rule, and its order

`modern/server/world/GotoService.cpp`, transliterated from `GLCharMsg.cpp:238-318`.
The order is load-bearing and is asserted as such:

1. **Dead character** refuses (`EM_ACT_DIE`), *before* anything is applied — a dying
   character refuses without its run state having changed.
2. **Run flag** applied. Exactly one bit. Deliberately *not* `MovementStateService`'s
   3032 rule, which touches four bits behind the `USER_GM3` gate; routing a 3034
   through that would grant a GOTO authority legacy never gave it.
3. **60-unit desynchronisation check** on the *full 3D* distance between the server's
   position and the client's claim, strictly `> 60`. The flag is applied first because
   that is the order legacy uses, and reordering it would change which requests are
   refused.
4. **No mesh** is its own outcome, distinct from "unreachable": RAN's field servers
   always have a mesh, so a modern server without one needs a different fix.
5. **±10 vertical probe** about the *destination*, and the **raw** requested target is
   what is stored — not the probe's resolution.
6. Speed is set only on the success path, through `002a`'s seam, so a 3034 and a 3032
   cannot disagree about what a character walks at.

### The wire contract is silence

A refused GOTO sends **nothing**. An unreachable destination, a dead character and a
desynchronised client are indistinguishable on the wire, and that is the measured
behaviour (`002c` section 10), not an omission here. Inventing a rejection packet would
be a new network message.

This is why `FieldConnection::PumpUntilGotoCount` takes a **count** rather than "until
the next one": a client waiting for "the next answer" would block forever on a refused
move.

## RAN has no per-tick position broadcast

`002c` section 10 measured that `GLChar::FrameMove` transmits nothing while a
character walks; the only position corrections are the event-driven `3064` and `3830`.
So `3035` arrives **once per accepted GOTO** and is the only position the server ever
sends. A client runs its own predicted walk and uses the 3035 to learn that the server
agreed — and, because the payload is the *server's* position, to learn where the server
thinks it is when the two have drifted.

Nothing in this milestone adds a per-tick broadcast, and that restraint is deliberate.

## Movement

`modern/core/movement/` holds a headless transliteration of `GLChar`:

- `Actor` takes **injected** elapsed seconds rather than reading a clock, so tests are
  deterministic and `max_distance = maxSpeed * injectedElapsedSeconds` is exact.
- The ±5/±10 probes, the `0.01f` arrival threshold and the `0.98f` friction on blocked
  motion are preserved as measured.
- `WorldMovementRuntime` owns one actor per session, guards each with its own mutex,
  and exposes `Tick(float)` so a test injects time instead of sleeping.

### The speed table

`modern/core/movement/MovementSpeed.h` carries RAN's recovered 16-entry table. RAN
indexes it by `EMCHARINDEX` — class **and** gender — which is why `WorldCharacter` grew
a `characterGender`. The 2333 wire layout is unchanged; the field is only used to
resolve a speed.

The wrong legacy constructor defaults (`12` and `34`) are **not** used. A test asserts
the table value by value and proves those defaults are absent, because a plausible wrong
constant is worse than an obvious zero.

`CharacterClassMovementSpeed` is wired into production through
`WorldServerRuntime`'s `MovementStateService`. Without that wiring the milestone would
have looked complete while every character walked at the `Actor` placeholder speed
regardless of class.

### Deferred on purpose

Equipment, vehicles, disguise, passive modifiers and ramp modifiers are **not**
implemented. They are real RAN behaviour and they are not in this milestone; the speed
seam is the single place they will go.

## Navigation behind it

`INavigationMapSource` is the seam. Production is `MapRegistryMeshSource` over a loaded
`MapRegistry` (`002e`), bound with `WorldServerRuntime::SetNavigationMapSource`.

Navigation meshes are immutable and shared; actors hold a `NavigationSearchSession`
each. `FieldRoleRuntime` stops the ticker **before** the listeners, so an actor can
never be ticked against a map source that has already gone away.

## Tests

| binary | new | what it proves |
|---|---|---|
| `ModernNetworkTests` | 15 | 3034/3035 byte layout, offsets, sizes, refusal of non-finite and malformed input |
| `ModernCoreTests` | 24 | the 16-entry table, the base-term formula, probing, arrival, wall slide |
| `ModernServerTests` | 34 | the 60-unit rule and validation **order**, the runtime, ticks, snapshots |
| `ModernWorldEntryTcpTests` | 5 | the real `2359 → 2333 → 3034 → 3035` exchange |

The 60-unit rule is asserted **at its boundary** (59 accepted, 61 refused) rather than
near it: a test that only tried 500 units would pass against a server whose threshold
was 10 or 5000.

### Two things the TCP tests deliberately do not claim

- **That the character keeps walking.** With no per-tick broadcast there is no
  on-the-wire evidence that time passed once the 3035 is read. Movement itself is proved
  headlessly, where elapsed time can be *injected* rather than waited for.
- **A malformed 3034.** The framer's handling of an unreadable frame is already covered
  generically, and a four-byte partial header blocks the server's read rather than being
  rejected as a malformed message — a different case with different semantics, and not
  one this milestone needs to invent.

### A note on test races

Server-side counters are written by the Field role's worker thread, and a client that
has already decoded the 3035 is one step ahead of the code that counts the send.
Asserting those counters straight after a client-side pump fails roughly one run in
five. Every such assertion here goes through `WaitFor`, which is the file's existing
answer to exactly this race — as it is for the two-client broadcast, where the server
skips a peer it has not yet marked spawned.