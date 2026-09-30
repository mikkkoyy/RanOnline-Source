# VERTICAL-004 Investigation Report — Codex Progress + Contribution

Read from `legacy/` before the implementation was written, then re-verified against
the same files afterwards. Every claim below is traced to a file and line. RAN
terminology is kept; nothing is borrowed from another MMORPG. Where the legacy
code does something the modern core deliberately does not, the difference is
stated rather than smoothed over.

The headline finding is §3: **almost none of the codex is live.** Of RAN's eleven
progress types, exactly one has a working implementation, and the machinery the
other ten imply has been commented out at both ends.

---

## 1. What a codex entry is

**Definition** is one row of RAN's codex table, `SCODEX_FILE_DATA`
(`legacy/Lib_Client/G-Logic/GLCodexData.h:26`). It is global table data, not
per-character state.

**Per-character record** is `SCODEX_CHAR_DATA`
(`legacy/Lib_Client/G-Logic/GLCodexData.h:179`), and there are **two** of them per
entry, not one:

```cpp
// legacy/Lib_Client/G-Logic/GLCharData.h — the character holds two maps
SCODEX_CHAR_DATA_MAP     m_mapCodexProg;   // in progress
SCODEX_CHAR_DATA_MAP     m_mapCodexDone;   // completed
```

Both are keyed by `DWORD` codex id, so an id is in exactly one of them at a time.
That invariant is what makes the reward exactly-once across a reload (§4), and it
is why the modern core models the state as two maps rather than one record with a
flag.

The five requirement slots are positionally significant and are copied in a
fixed order by `Assign` (`GLCodexData.cpp:388-407`):

| Slot | Item id | Quantity | Done flag |
|------|---------|----------|-----------|
| 0 | `sidMobKill`  | `wQuantity1` = `wProgressMobKill`  | `dwProgressItemDone1` |
| 1 | `sidMapKill`  | `wQuantity2` = `wProgressMapKill`  | `dwProgressItemDone2` |
| 2 | `sidMapReach` | `wQuantity3` = `wProgressMapReach` | `dwProgressItemDone3` |
| 3 | `sidItemGet`  | `wQuantity4` = `wProgressItemGet`  | `dwProgressItemDone4` |
| 4 | `sidItemUse`  | `wQuantity5` = `wProgressItemUse`  | `dwProgressItemDone5` |

The legacy slot names are dropped in the modern core. "Mob kill" and "map reach"
name rules that nothing implements (§3), and `CodexRequirement` keeps only what
the live rule reads: the item, the count, and the grade.

## 2. The required-count cascade

`SCODEX_CHAR_DATA::Assign`, `GLCodexData.cpp:377-386`, verbatim:

```cpp
// legacy/Lib_Client/G-Logic/GLCodexData.cpp:377
dwProgressMax = 5;
if ( sactivity_file_data.wProgressItemUse == 0 )
    dwProgressMax = 4;
if ( sactivity_file_data.wProgressItemGet == 0 )
    dwProgressMax = 3;
if ( sactivity_file_data.wProgressMapReach == 0 )
    dwProgressMax = 2;
if ( sactivity_file_data.wProgressMapKill == 0 )
    dwProgressMax = 1;
```

Three properties of this are load-bearing, and all three are reproduced rather
than tidied up, because `dwProgressMax` is the rule that decides when an entry
completes — changing it makes modern entries complete at a different moment than
RAN's would.

**The `if`s are sequential assignments, not a chain of early returns.** The last
one that fires wins. For a table filled left to right this gives the same answer
as an early return, because the unconfigured slots are the high-numbered ones and
the lowest unconfigured slot is tested last. `CodexDefinition::RequiredSlotCount`
is written the same way for that reason.

**Slot 0 is never tested.** The cascade tests slots 4, 3, 2 and 1. Slot 0 —
`wProgressMobKill` — is the one slot it skips. A definition configuring *only*
slot 0 therefore reports a required count of 1, not 5, because slot 1 being
unconfigured is what sets it. This is harmless for the prefix-shaped data RAN's
table holds; it is not harmless for the one shape RAN never ships, a definition
with slots 1–4 configured and slot 0 not, which reports 5 and can never complete.
`CodexTests.cpp` pins the working shapes and does not assert the broken one.

**No configuration still returns 1, never 0.** An entry naming no items reports a
required count of 1 and can never be completed, because there is no slot 0
requirement for a registration to match. RAN seats such an entry and leaves it
there forever. Modern seats it too — the seating matches RAN — but refuses
registration with `NotCompletable` (§9) rather than reporting a successful
registration that matched nothing.

## 3. Only one of the eleven progress types is live

This is the central finding, and it is why VERTICAL-004 models a codex as "register
an item against a table" rather than as a progression system.

RAN's table has eleven progress types, and each one has a `dwProgressMax`
assignment in `Assign` and a matching recomputation in `Correction`. **Both of
those switches are inside block comments**:

- `Assign`'s spans `GLCodexData.cpp:415-480`, opening at `:415`
  (`/*switch ( emType )`) and closing at `:480` (`}*/`).
- `Correction`'s spans `GLCodexData.cpp:508-585`, opened and closed the same way.

The per-type rules — "reach level 40", "kill 12 mobs", "finish quest X" — are not
compiled. What that leaves live in `Assign` (`:371-481`):

- the five item ids, five quantities and five grades, plus `dwCodexID` and the
  type (`:388-407`)
- the `dwProgressMax` cascade of §2, uncommented, at `:377-386`
- `dwProgressNow = 0` (`:400`)

And what that leaves live in `Correction` (`:483-…`):

- the type-change reset at `:486-494`, which is **not** commented out: on a type
  mismatch it clears `sidProgress`, `dwProgressNow` and `dwProgressMax`, re-runs
  `Assign`, and returns (`:492-493`)
- the five item-id refreshes at `:497-506`

So the only way `dwProgressNow` moves is item registration, and a type change
does discard progress. What is *not* live is `Correction`'s per-type
`dwProgressMax` recomputation (`:508-585`), which is why a same-type retune
leaves `dwProgressMax` stale (§7).

Modern consequence, stated as a decision rather than a gap: `CodexType` is
retained because it selects the reward field (§5) and the snapshot needs it to
pick an icon, but no type has a progress rule. `ServerCharacter::RegisterCodexItem`
is the only mutator, and its own comment says so. The type-change reset *is*
reproduced, because it is live.

## 4. Registering an item

`GLChar::DoCodexRegisterItem`, `GLCharCodex.cpp:60`. The signature carries the
three things it matches on:

```cpp
// legacy/Lib_Client/G-Logic/GLCharCodex.cpp:60
void GLChar::DoCodexRegisterItem( SNATIVEID sidItem, DWORD dwCodexID,
                                  WORD wTurnNum, WORD wGrade )
```

The per-slot arm, `:75-90`, is the whole rule:

```cpp
// legacy/Lib_Client/G-Logic/GLCharCodex.cpp:75
if ( scodex_char_data.sidProgressItem1 == sidItem )
{
    if ( scodex_char_data.wItemGrade1 == wGrade )
    {
        if ( scodex_char_data.dwProgressItemDone1 != 1 )
        {
            if ( scodex_char_data.wQuantity1 == wTurnNum )
            {
                scodex_char_data.dwProgressItemDone1 = 1;
                scodex_char_data.dwProgressNow++;
            }
        }
    }
}
```

Four properties, all reproduced:

- **Item id is compared for equality.** No prefix, no family, no category.
- **Quantity is compared for equality**, not `>=`. A stack of 2 does not satisfy a
  requirement of 3, and a stack of 4 does not satisfy a requirement of 3 either.
  Relaxing this to `>=` would be an invention, and it would let one registration
  satisfy a five-count requirement.
- **Grade is compared for equality**, against the *registered instance's* grinding
  grade, not the table's item grade.
- **The five arms are independent `if`s, not `else if`.** One stack can satisfy
  two slots when an entry names the same item twice, and both are counted.

Completion, `:147-173`:

```cpp
// legacy/Lib_Client/G-Logic/GLCharCodex.cpp:147
if ( scodex_char_data.dwProgressNow >= scodex_char_data.dwProgressMax )
{
    scodex_char_data.dwProgressNow = scodex_char_data.dwProgressMax;
    vecComplete.push_back( scodex_char_data.dwCodexID );
}
```

The counter is **clamped**, so it can never exceed the required count, and the
entry is then moved from the progress map to the done map in a **second pass**
(`:161-173`) — `CodexComplete` inserts into `m_mapCodexDone` (`:19`) and
`m_mapCodexProg.erase` removes it. Modern does the same two-phase move, which is
why a single registration can complete at most the entries it names, and why the
loop over the whole progress map cannot be short-circuited by a completion.

`CodexComplete` itself (`GLCharCodex.cpp:14`) is not a bare insert, and one of
its early exits matters:

```cpp
// legacy/Lib_Client/G-Logic/GLCharCodex.cpp:16
SCODEX_FILE_DATA* pcodex_file_data = GLCodex::GetInstance().GetCodex( scodex_char_data.dwCodexID );
if ( !pcodex_file_data )	return;
```

If the table row has gone, the function returns **without inserting into the done
map** — and the caller has already decided to complete, so the
`m_mapCodexProg.erase` at `:171` still runs. The entry is in neither map: a
completed codex entry silently disappears. The rest of the function is transport
and notification (`:21-27`, `:29-…`), all out of scope; no badge is granted here
(§8). Modern reports the skip instead (§5), which is the one place a missing
definition is visible rather than silent.

## 5. The reward mapping

`GLCHARLOGIC::CODEX_STATS`, `GLogixExPC.cpp:5103`. The function's shape, in order:
eleven `DWORD` accumulators are zeroed (`:5106-5116`), the **done** map is walked
in full (`:5122-5123`), each entry's `dwRewardPoint` is added to the accumulator
its `emType` selects (`:5133-5154`), the results are copied to the character
(`:5158-5168`), and `INIT_DATA(FALSE,FALSE)` runs (`:5172`).

The mapping, transcribed exactly:

| `emType` | Accumulator | Statistic |
|----------|-------------|-----------|
| `EMCODEX_TYPE_REACH_LEVEL`      | `m_dwHPIncreaseDummy`        | HP |
| `EMCODEX_TYPE_KILL_MOB`         | `m_dwMPIncreaseDummy`        | MP |
| `EMCODEX_TYPE_KILL_PLAYER`      | `m_dwSPIncreaseDummy`        | SP |
| `EMCODEX_TYPE_REACH_MAP`        | `m_dwAttackIncreaseDummy`    | Attack |
| `EMCODEX_TYPE_TAKE_ITEM`        | `m_dwDefenseIncreaseDummy`   | Defense |
| `EMCODEX_TYPE_USE_ITEM`         | `m_dwShootingIncreaseDummy`  | Shooting |
| `EMCODEX_TYPE_REACH_CODEX`      | `m_dwMeleeIncreaseDummy`     | Melee Power |
| `EMCODEX_TYPE_COMPLETE_QUEST`   | `m_dwEnergyIncreaseDummy`    | Magic Attack |
| `EMCODEX_TYPE_CODEX_POINT`      | `m_dwResistanceIncreaseDummy`| Resistance |
| `EMCODEX_TYPE_QUESTION_BOX`     | `m_dwHitrateIncreaseDummy`   | Hit Rate |
| `EMCODEX_TYPE_ETC`              | `m_dwAvoidrateIncreaseDummy` | Avoid Rate |

Four things about the function that are not obvious from reading it:

- **It is a full recompute, not an accumulation.** The accumulators are local
  `DWORD`s reset on entry, and the results are *assigned* at `:5158-5168`. Calling
  it twice with the same done set gives the same answer, and calling it four times
  gives the same answer four times.
- **The two parameters are unused.** `nIndex` and `dwStatPoint` (`:5103`) are
  never read. A caller cannot ask for a partial recompute, and a caller cannot
  inject a value.
- **Only the done map is walked.** Progress contributes nothing, however far
  along it is.
- **A missing definition is silently skipped.** `if ( pcodex_char )` at `:5131`
  guards the whole switch; an entry whose table row has gone contributes nothing
  and is not reported. Modern keeps the skip but makes it *visible* through
  `CodexContributionError` (§9), because a server that cannot say why a reward is
  missing cannot fix it.

`Stats::CodexContribution` already existed in the modern core and is already
consumed by `Stats::Calculate`. VERTICAL-004 adds `CodexContributionAggregator`
and nothing else — no formula is restated.

## 6. The item is spent whether or not the entry matched

The request handler is `GLCharInvenMsg.cpp:9080-9213`. Its order of operations:

1. The completed map is checked first, and a hit is refused as
   `EMREQ_REGISTER_CODEX_FB_INVALIDITEM` (`:9130-9136`). **An already-completed
   entry rejects the request before the item is looked at**, so a completed entry
   cannot be re-paid by resubmitting its item.
2. The progress record is checked for a *satisfied* slot naming the same item
   (`:9143-9177`), and if any is already done the request is refused with the same
   code (`:9180-9185`).
3. Otherwise `DoCodexRegisterItem` runs (`:9187`).
4. Then, **unconditionally**, the item is routed for deletion (`:9195`) and removed
   from the inventory (`:9199`).

Step 4 is the bug. `DoCodexRegisterItem` returns `void` and reports nothing, so
the handler has no way to know whether anything was recorded. Registering an item
that no codex entry names **destroys the item and credits nothing**.

Modern contract: `RegisterCodexItem` does not touch an inventory, because there
is none in this milestone, and it returns `CodexRegistration::recorded` so a
future inventory-owning caller knows whether to spend. A caller that ignores that
flag and spends unconditionally reproduces RAN's data loss; the flag exists
precisely so it does not have to.

The two refusals in steps 1 and 2 are both `INVALIDITEM`, but they are different
facts — the entry is finished, versus this particular slot is already satisfied —
and modern reports them distinctly as `AlreadyCompleted` and
`NoMatchOnASatisfiedSlot`.

## 7. Reconciliation at load

`GLCharDataCodex.cpp:73-132`. The order is: walk the definition table; skip
anything already in the done map (`:79-80`); `Correction` an existing progress
record (`:86`); otherwise `Assign` and seat it (`:91-92`); then a second pass that
erases from **both** maps any record whose definition has gone (`:97-132`).

Two details:

- **The `continue` at `:79-80` is the whole exactly-once guarantee.** A completed
  entry is never re-seated, so loading a character cannot make it completable
  again. Modern keeps it, and it is the reason `ReconcileCodex` after a payment
  does not change the contribution.
- **A type change resets the record, and that code is live.** `Correction`'s
  type-mismatch branch (`GLCodexData.cpp:486-494`) clears the counters and re-runs
  `Assign`; it sits outside the commented-out switch, so it runs. Modern resets on
  a type change for the same reason, and a done flag referring to a requirement
  the new type does not have is not a claim worth keeping.

The same-type branch of `Correction`, `:497-506`, refreshes only the five item
ids. It leaves the captured quantities, the captured grades and `dwProgressMax`
stale — the per-type recomputation that would have fixed the last of those is
inside the commented switch (`:508-585`). Modern refreshes the whole requirement
and the required count — a documented divergence (§9) — and, unlike RAN, only
discards progress when a requirement actually changed, because a done flag is a
claim about a requirement rather than about a record.

## 8. Badges and notifications are not awarded

`SCODEX_FILE_DATA` carries `strBadgeString` and `bRewardBadge`
(`GLCodexData.h:32`, `:39`). Within the codex system they are read only to
**display** a definition's badge beside a completed entry.

The code that would *grant* a badge exists only in the separate Activity system
(`GLCharActivity.cpp:519-533`, `GLCharactorReq2.cpp:733-739`), and
`GLChar::CodexComplete` (`GLCharCodex.cpp:14-…`) has no equivalent — after its
definition lookup it inserts, sends a message, broadcasts, and dispatches a
notification. The `DoCodexReachPoint` call at `GLCharCodex.cpp:177` is itself
commented out.

Modern: `CodexDefinition` carries `badge` and `rewardBadge`, and
`CharacterSnapshot::CodexEntry` publishes the badge string, because a codex panel
needs to draw it. Nothing grants it. That is presentation, not a reward, and the
field is named so a reader cannot mistake it for one.

Notification types are carried on the definition and not acted upon. The transport
for them (`SNETPC_CODEX_UPDATE`, `GLCharCodex.cpp:154-156`) is a network
message, and networking is out of scope for this milestone.

## 9. Divergences, and why

| Aspect | RAN | Modern | Reason |
|--------|-----|--------|--------|
| Per-type progress rules | Commented out at both ends | Not implemented | §3. Not modelled rather than guessed at. |
| Required-count cascade | Four sequential `if`s, last match wins | Same shape | §2. It is the completion rule. |
| Slot 0 untested | Yes | Yes | §2. Same answer for RAN's data. |
| Empty entry seated | Yes, forever un-completable | Seated, registration refused `NotCompletable` | The seating matches RAN; the refusal makes a broken table row visible instead of a silent no-op. |
| Grade match | Exact, against the instance | Field kept, **not enforced** | `ItemInstance` carries no grade, upgrade or option state by design. Enforced when per-copy state exists. |
| Quantity match | `==` | `==` | §4. Relaxing it would be an invention. |
| Item spend | Unconditional delete | Not spent; `recorded` returned | §6. Reproduces the data loss. |
| Recompute | Full, parameters unused | Full, no parameters | §5. |
| Missing definition | Silently skipped | Skipped, reason reported | §5. A server cannot fix what it cannot see. |
| Reconciliation refresh | Ids only | Whole requirement + count | §7. A retune must not leave a stale count. |
| Progress across refresh | Kept always | Kept if unchanged, dropped if not | A done flag is a claim about a requirement. |
| Badge grant | Never, in the codex system | Never | §8. |
| Client recompute | Client keeps its own mirror and calls `CODEX_STATS` | Snapshot only | A second authority is a second answer. |

## 10. Deferred and not modelled

- **Inventory.** There is none. `RegisterCodexItem` takes an `ItemInstance` as an
  argument and does not own or consume it.
- **Networking.** `SNETPC_CODEX_UPDATE` and `SNETPC_CODEX_CHAR` are transport.
- **Persistence.** The two maps are not serialised; RAN's `ByteStream <<`
  round-trip at `GLCharDataCodex.cpp:19-29` is not transcribed.
- **The codex table format.** RAN reads a `.csv` through `GLCodex::LoadFile`
  (`GLCodex.cpp:97`) and `GLCodex::LoadCsv` (`GLCodex.cpp:195`). Modern takes a
  `CodexDefinitionProvider`; loading the table is a later milestone, which is why
  `InMemoryCodexDefinitions` is the only implementation.
- **Per-type progress.** §3. Deliberately absent, not deferred behind a stub.
- **`m_dwCodexDoneSize`.** Commented out at `GLogixExPC.cpp:5118-5120`, and the
  field it guarded is not modelled.

## 11. Modern architecture mapping

```
CodexId                    strong id, distinct from ItemId and SkillId  [new, core]
CodexDefinition            id, type, title, description, badge, notify,
                           rewardPoint, 5 requirement slots              [new, core]
CodexRequirement           item, quantity, requiredGrade                [new, core]
CodexDefinitionProvider    read-only Find + GetAll
                           + InMemoryCodexDefinitions                   [new, core]
CodexState                 two ordered maps (progress, completed),
                           Reconcile + RegisterItem, a value type        [new, core]
CodexRegistration          error, recorded, completed, doneCount,
                           requiredCount - the caller-facing outcome     [new, core]
CodexContributionAggregator completed set + provider
                           -> Stats::CodexContribution                  [new, core]
ServerCharacter            + CodexState, owns it, recalculates on
                           every payment                                 [extended]
CharacterSnapshot          + CodexList (flat, sorted, id/type/name/
                           description/badge/completed/doneCount/
                           requiredCount)                                [extended]
ClientCharacterState       + read-only codex views                      [extended]
```

The authority split is the point. The server owns the two maps, decides
completability, aggregates the contribution and publishes a flat list. The client
holds that list, has no codex contribution of its own, and no path to producing a
third number — the structural guarantee `CharacterSnapshot::CodexEntry` gets for
free by carrying no reward values at all. RAN's client breaks that guarantee for the codex specifically: it keeps its own
`m_mapCodexProg` / `m_mapCodexDone` mirror, erasing from the first and inserting
into the second on the completion message (`GLCharacterMsg.cpp:5206-5212`), and
calls its own `CODEX_STATS` right after (`GLCharacterMsg.cpp:5214`). The formula
therefore runs on both sides and the client maintains a second answer; that is the
one place the legacy design has two sources of truth, and it is not reproduced.

`ServerCharacter::SetContributions` refuses a non-zero codex contribution, on the
same terms as the item and passive arguments it already refused: the completed
codex set is the only source, and a caller told "ok" whose contribution never
appears has no way to notice.

## 12. Where the tests are

`modern/tests/CodexTests.cpp` covers the transcription directly, in the core
library, with no server and no client:

- the cascade at every prefix length, the untested slot 0, and the empty entry
- registration matching id and quantity for equality, including a too-small and a
  too-large stack
- one stack satisfying two slots naming the same item, and a satisfied slot not
  being recorded twice
- a slot outside the required count never being consulted
- the counter clamp, and the two-phase move out of the progress map
- `UnknownCodex`, `AlreadyCompleted`, `NotCompletable` and the bad-argument cases
  being four distinguishable outcomes
- reconciliation: seating, the completed-entry skip, the type-change reset, the
  quantity/count refresh, progress preserved across an identical refresh and
  dropped when a requirement changes, and orphan removal from both maps
- aggregation: only completed entries, per-field accumulation, idempotence,
  order independence, a skipped entry reported, and an unmapped type reported

The server-side consequences are in `modern/server/ServerCharacterTests.cpp`:
seating on create, a registration moving the derived statistics and matching an
independent `Stats::Calculate`, only-a-completed-entry-pays, the reward paid
exactly once, reconciliation not repaying, the aggregate not drifting across four
reloads, per-type mapping at the server boundary, the published snapshot carrying
the authoritative counters, repeated snapshots identical, no provider being inert,
an empty table being refused, and a rejected registration leaving the character
byte-for-byte intact.

The client's read-only behaviour is in
`modern/client/gameplay/ClientGameplayTests.cpp`: presentation from a snapshot,
the finished and unfinished cases being distinguishable, the views being
const-refs over the published list, the client's derived statistics being the
server's, the empty fallbacks, and a later snapshot replacing an earlier codex
rather than merging with it.

See §13.7 of `docs/MODERN_ARCHITECTURE.md`.
