# Vendored dependency: miniLZO 2.10

This directory contains **unmodified** third-party source. Nothing here was edited,
patched or regenerated. Read-only: any local change would invalidate the hash record
below and the upstream provenance.

## Provenance

| Field | Value |
| --- | --- |
| Component | miniLZO — "mini subset of the LZO real-time data compression library" |
| Version | 2.10 (`MINILZO_VERSION 0x20a0`) |
| Upstream | Markus Franz Xaver Johannes Oberhumer <markus@oberhumer.com> |
| Canonical URL | `http://www.oberhumer.com/opensource/lzo/download/minilzo-2.10.tar.gz` |
| Archive SHA-1 | `c7432708d49017a3f0b4f44c99d336f8a1be84f5` |
| SHA-1 published by upstream | `c7432708d49017a3f0b4f44c99d336f8a1be84f5` — **verified to match** |
| Retrieved | VERTICAL-030 |
| Archive | `minilzo-2.10.tar.gz`, 63,314 bytes |

The archive hash was compared against the SHA-1 published on
`http://www.oberhumer.com/lzo/` before extraction. They match.

### File hashes (post-extraction, as committed)

| File | Bytes | SHA-1 (first 16 hex) |
| --- | ---: | --- |
| `minilzo.c` | 213,674 | `019debb32718813c` |
| `minilzo.h` | 3,089 | `2525efd63ffab0c0` |
| `lzoconf.h` | 16,006 | `4d65714c54dc62ad` |
| `lzodefs.h` | 127,289 | `e2f4efc645614ce1` |
| `README.LZO` | 4,003 | `697b04aa839f3ec6` |

`testmini.c` from the archive is **not** vendored: it is upstream's standalone
self-test and duplicates coverage that the project's own tests provide.

`README.LZO` is included because upstream requires it: *"Note: you also must distribute
this file ('README.LZO') with your project."*

## Licence — read this

miniLZO is distributed under the **GNU General Public License, version 2 or later**,
as stated in the header of `minilzo.c` and `minilzo.h`:

> The LZO library is free software; you can redistribute it and/or modify it under the
> terms of the GNU General Public License as published by the Free Software Foundation;
> either version 2 of the License, or (at your option) any later version.

**This is flagged as an open legal question, not as a settled one.** LZO has a long
history of also being offered under terms permitting use in non-GPL applications, but
that allowance is **not** stated in the distributed miniLZO headers, and this repository
does not currently ship a `COPYING` or a project-level licence statement. Whether
embedding unmodified miniLZO here is compatible with this project's licensing is a
determination for the project owner, not something this repository can assert.

Consequences of the choice made in VERTICAL-030:

- The files are vendored **byte-identical to upstream**, deliberately. Upstream's
  `README.LZO` notes miniLZO is generated automatically from the LZO sources and that
  "functionality is completely identical"; keeping it unmodified preserves both the
  audit trail and the strongest possible reading of any redistribution rights.
- No GPL-derived code was refactored into project style, and no LZO code was merged
  into `CompressionCodec`. The boundary is a narrow project-owned interface
  (`modern/network/CompressionCodec.h`) with LZO confined to the `.cpp` behind it.

If the project owner determines this is unacceptable, the supported replacement path is
documented in `docs/reference/network/MODERN_NET_COMPRESS_LZO.md`: the codec is reached
only through `Lzo1xCodec`, so swapping the backend does not touch the envelope codec,
the batching state machine, or their tests.

## Why miniLZO and not `lzo2.lib`

The repository already contained `legacy/Tik/Library/lzo2.lib` — RAN's own prebuilt
LZO2 static library. It was rejected as the modern backend:

1. It is a **VC7.1 (Visual Studio 2003) binary** with no corresponding source in the
   tree. Linking it under MSVC 14.x fails with `LNK2019: unresolved external symbol
   __except_handler4_common` unless `/NODEFAULTLIB:MSVCRT` is used to suppress the
   legacy CRT. With that override a round-trip does succeed, but a build-wide CRT
   override is a poor trade for a codec.
2. It would make `ModernNetwork` depend on `legacy/`, which
   `modern/network/CMakeLists.txt` forbids in its own header comment and which the
   root `CMakeLists.txt` deliberately avoids.
3. A binary blob with no auditable source is a worse dependency than the ~30 KiB of
   source that upstream distributes for this exact purpose.

### Scope, corrected by VERTICAL-030-A

This file originally justified the rejection by citing `GameClient2.vcproj`, which
implied `lzo2.lib` is a client-side concern. **It is not.** VERTICAL-030-A enumerated
**25 consuming projects** — all four servers, `GameEmulator`, `GameViewer`, `GMTool`
and every editor. `Lib_Network` does not link it directly; it compiles `MinLzo.cpp`,
leaving `lzo1x_*` unresolved inside `Lib_Network.lib`, and each executable resolves the
dependency at final link. `lzo2.lib` is therefore a **transitive dependency of the entire
RAN tree**, and the `/NODEFAULTLIB:MSVCRT` problem would have applied to all 25 links.

### The decisive fact, established by VERTICAL-030-A

**No LZO implementation source exists anywhere in the repository.** A whole-tree search
of 3,580 files found LZO only in project files (as `lzo2.lib` dependency entries), the
`Lib_Network` wrapper, and the `Tik/Include` header set. There is no LZO `.c` file in
the tree at all, and no file named `minilzo.h` — even though
`legacy/Lib_Network/MinLzo.cpp:2` does `#include "minilzo.h"`.

So `Lib_Network` cannot compile its own LZO wrapper as committed, and vendoring LZO
*source* was the only option available rather than merely the tidiest one. `lzo2.lib` is
itself built from 74 `src/*.c` files that are not in the tree.

Full findings: `docs/reference/source/RAN_CLIENT_TOOLS_SOURCE_INVENTORY.md` §1, §5.

miniLZO implements the same `lzo1x_1_compress` and `lzo1x_decompress_safe` entry
points that legacy `CMinLzo` calls (`legacy/Lib_Network/MinLzo.cpp:112`, `:158`), so
the wire format is unaffected by the substitution.