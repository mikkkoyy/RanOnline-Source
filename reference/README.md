# reference/

Research material: data-format exports, mapping tables, protocol notes and audit
output. **Nothing in this directory is built, and nothing in it ships.**

The modern implementation is the authoritative one. This directory exists so
that what we learned about the legacy formats is written down instead of being
rediscovered, and so that the conversions are auditable.

| Directory         | What lives there                                                              |
| ----------------- | ----------------------------------------------------------------------------- |
| `data-formats/`   | CSV exports of legacy tables and native round-trip validators                 |
| `legacy-mapping/` | Legacy concept -> modern type mapping                                          |
| `protocol/`       | Packet and wire-format notes                                                   |
| `notes/`          | Audits, dependency register, and one-off research scripts                     |

Each subdirectory has its own `README.md`. Start with
[notes/modern-legacy-dependencies.md](notes/modern-legacy-dependencies.md) if
you are looking for the rules the modern tree has to obey.
