# protocol/

Packet and wire-format notes. **Nothing here has been written up yet.**

This directory is reserved for the reverse-engineering output that the network
layer will need: packet headers, field order, versioning, and the `Tik` /
GGAuth framing.

## Status

No protocol documentation exists in the repository yet. It will be written
while `modern/network` is built, by reading the legacy implementation in
`legacy/Lib_Network` and `legacy/Tik`.

The expected shape of a note in this directory, once written:

- packet id and direction;
- fixed header fields, with sizes and byte order;
- payload layout;
- the legacy file and function the layout was read from;
- known client-version variants.

## Related

- Legacy mapping of the ODBC / SQL layer: [`../notes/`](../notes/).
- Legacy type mapping: [`../legacy-mapping/`](../legacy-mapping/).
- Architecture and layering rules: [`../../docs/MODERN_ARCHITECTURE.md`](../../docs/MODERN_ARCHITECTURE.md).
