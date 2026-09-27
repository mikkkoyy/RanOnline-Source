# data-formats/

Exports and validators for legacy RAN binary data. Not built, not shipped.

| File / directory                     | What it is                                                                              |
| ------------------------------------ | --------------------------------------------------------------------------------------- |
| `Item.csv`                          | Headered export of the legacy item table. Row 2 onward is data. The header names the legacy C struct and member for every field (`sNativeID wMainID`, `gdDamage wLow`, ...), so the CSV is self-describing. |
| `RandomOpt.csv`                      | Item-ID -> random-option-table mapping, in the form the legacy loader consumes.          |
| `NativeItemRoundTrip/`               | Native round-trip validator for the `.isf` item file: read -> write -> read, comparing the two. |

## NativeItemRoundTrip

`NativeItemRoundTrip.cpp` validates that the legacy `.isf` item file can be
parsed and re-emitted without losing or corrupting fields. It is the check that
lets us treat `.isf` as a format we understand before we depend on it.

It writes its fixtures and logs to `ItemTest/` at the repository root:

- `item_orig.isf` — bytes as read
- `item_original.isf` — untouched input
- `item_roundtrip.isf` — bytes after the round trip
- `Logs/` — tool output

`ItemTest/` is local scratch data (~72 MB) and is deliberately **not** committed
(see `.gitignore`). Regenerate it by running the tool locally; do not add it.

## Related

- Where the modern side consumes these: `modern/core/item/ItemData.h`,
  `modern/core/progression/ProgressionData.h`.
- EXP table export contract: `modern/core/data/README.md`, produced by
  `modern/tools/exptable_dump.cpp`.
