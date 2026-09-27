# Modern -> Legacy Dependencies

The project rule is that `modern/` never depends on `legacy/`. See
[docs/MODERN_ARCHITECTURE.md](../../docs/MODERN_ARCHITECTURE.md).

This file is the register of every place that rule is knowingly broken, so the
exceptions stay visible instead of quietly accumulating.

## Register

| # | Location                                                        | Kind                     | Depends on                                      | Why                                                                                     | Status        |
| - | --------------------------------------------------------------- | ------------------------ | ----------------------------------------------- | --------------------------------------------------------------------------------------- | ------------- |
| 1 | `modern/compatibility/legacy/` (source present, not built)       | sanctioned bridge         | `legacy/Lib_Client` (link) and `GLChar.h` (include) | Adapters are the designed place to meet legacy types. Isolated in its own static lib, never linked into `core`. | deferred     |
| 2 | `modern/tools/exptable_dump.cpp`                                 | research tool           | `legacy/Lib_Engine/Common/ByteCryptDef.h`, `ByteCryptDefVer1.h`, `ByteCryptDefVer2.h` | The legacy EXP table is packed and encrypted; the unpacker is the only existing definition of the format. The tool is offline-only and is not part of any shipped target. | accepted      |

## Notes

### 1. `ModernLegacyAdapter` — deferred, not built

- Lives in `modern/compatibility/legacy/`, which the architecture doc designates
  as the only modern layer allowed to see legacy code.
- **It is not in the build.** `modern/CMakeLists.txt` does not add the
  subdirectory as of CORE-001, for two reasons: legacy compatibility is out of
  CORE-001's scope, and `CharacterAdapter` maps `GLChar` fields — HP/MP/SP
  pools, the `ActState` bitfield, die/revive — that the clean `Character`
  deliberately does not have. It needs rewriting, not patching.
- Because nothing links it, the root `CMakeLists.txt` no longer adds
  `legacy/Lib_Helper`, `Lib_ZLib`, `BugTrap`, `Lib_Engine`, `Lib_Network`,
  `Lib_ClientUI` or `Lib_Client`. The CMake build touches no legacy library at
  all now.
- The legacy tree itself is unaffected and still builds through
  `legacy/RanOnline.sln`.
- When it comes back it keeps the same shape: public surface in modern types
  only, legacy types confined to the translation unit that converts.

### 2. `exptable_dump`

- The EXP table (`exptable_max.bin`) is stored in the legacy packed/encrypted
  format. The `ByteCryptDef*` headers are the only authoritative description of
  that container, so re-implementing the unpacker would be guesswork.
- Once the format is understood well enough, the tool should grow its own
  minimal header in `modern/tools/` and drop the legacy include. That is a
  follow-up task, not part of this restructure.
- Until then this exception is recorded here and referenced from
  `modern/tools/CMakeLists.txt`.

## What is not an exception

Comments in `modern/core` frequently say "legacy" — they cite the legacy
behaviour the code reproduces. A comment is a reference, not a dependency. The
check that matters is: does `modern/core` include or link anything from
`legacy/`? It does not.

`reference/legacy-calculation-port/` is a port of legacy *formulas* into
modern C++ types. It is reference material rather than an exception, because it
is not in any build target and cannot reach a shipped binary.
