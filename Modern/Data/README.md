# Modern Data Files

This directory contains portable text exports of RAN binary data files.

## EXP Table

The canonical RAN EXP table is stored in `exptable_max.bin` (packed/encrypted RAN format).
To generate the portable text export `exptable_max.txt`:

1. Build the legacy client/server with GLCONST_CHAR::EXP_MAX_LOADFILE
2. Run the loader to populate lnEXP_MAX_TABLE
3. Dump the table to text: one int64 per line, index = level (0..MAX_LEVEL-1)
   - Line 0: placeholder (level 0, typically 0 or LLONG_MAX)
   - Line 1: EXP needed for level 1 -> 2
   - Line N: EXP needed for level N -> N+1
   - Up to MAX_LEVEL (300) or wMAX_LEVEL (configurable, default 255)

## Base Character Data

Future: charclass.txt, school.txt, etc.