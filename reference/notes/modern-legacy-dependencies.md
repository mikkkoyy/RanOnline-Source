# Modern -> Legacy Dependencies

The project rule is that `modern/` never depends on `legacy/`. See
[docs/MODERN_ARCHITECTURE.md](../../docs/MODERN_ARCHITECTURE.md).

This file is the register of every place that rule is knowingly broken, so the
exceptions stay visible instead of quietly accumulating.

## Register

| # | Location                                                        | Kind                     | Depends on                                      | Why                                                                                     | Status        |
| - | --------------------------------------------------------------- | ------------------------ | ----------------------------------------------- | --------------------------------------------------------------------------------------- | ------------- |
| 1 | `modern/compatibility/legacy/` (`ModernLegacyAdapter` CMake target) | sanctioned bridge  | `legacy/Lib_Client` (link) and `GLChar.h` (include) | Adapters are the designed place to meet legacy types. Isolated in its own static lib, never linked into `core`. | by design     |
| 2 | `modern/tools/exptable_dump.cpp`                                 | research tool           | `legacy/Lib_Engine/Common/ByteCryptDef.h`, `ByteCryptDefVer1.h`, `ByteCryptDefVer2.h` | The legacy EXP table is packed and encrypted; the unpacker is the only existing definition of the format. The tool is offline-only and is not part of any shipped target. | accepted      |

## Notes

### 1. `ModernLegacyAdapter`

- Lives in `modern/compatibility/legacy/`, which the architecture doc designates
  as the only modern layer allowed to see legacy code.
- Links `Lib_Client` privately. `modern/core` links only `Modern`, so the legacy
  dependency cannot reach the core through the link graph.
- The public surface is modern types only (`Character`, `Vector3`, `Types.h`).
  Legacy types appear only in the `.cpp` translation unit that performs the
  conversion.
- The root `CMakeLists.txt` therefore still adds `legacy/Lib_Helper`,
  `Lib_ZLib`, `BugTrap`, `Lib_Engine`, `Lib_Network`, `Lib_ClientUI` and
  `Lib_Client`. That is a consequence of this one adapter, not a modern
  dependency.

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
