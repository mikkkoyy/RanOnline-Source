# WORLD-ENTRY-002c dependency resolution: map binding, speed, WLD crypt, navigation kernel, movement model

**Status: READY FOR IMPLEMENTATION.**

Baseline `73466eb` (WORLD-ENTRY-002b). Investigation only: no production movement code, no
change to any 002a behaviour, no weakened assertion, no regression.

002b left five blockers. **All five are now resolved, and two of them were resolved by
finding data that 002b had already looked for and missed** — the map index, and the class
speed table. The remaining work is a real but bounded port, not a discovery problem.

---

## 0. What changed versus 002b, in one table

| 002b blocker | 002b said | 002c measured |
|---|---|---|
| map → `.wld` | no `.glmap` exists; binding unavailable | **`mapslist.mst` exists** (11,494 B) and resolves **99/99** registered maps to a `.lev` and then to a `.wld` |
| `.wld` navmesh | 63/87 parse | **77/87 parse, 0 anomalies**; all 99 registered maps have one |
| obfuscated `.wld` | "not yet decoded" | **14/14 decoded**, 92,967 cells, 0 anomalies. One needed **two** passes. |
| `default.charclass` | present, keys in source, decoder blocked by a legacy PCH | **decrypted.** Keys need a transform `Rijndael.cpp:975-981` applies and 002b did not. Base speed recovered for all 16 classes |
| equipment speed | "no candidate data identified" | **731 items carry `EMVAR_MOVE_SPEED`** in `item.csv`; the modifier magnitude is in shipped data. Only the *worn set* is runtime state |
| navmesh port size | ~2,890 lines, D3DX9 + whole map engine | **~2,236 logical lines**, and the D3DX9 surface is **math only** — no `IDirect3DDevice`, no `DxLandMan`, no terrain |

The single most consequential correction: **002b's central blocker was a lookup failure, not
a missing capability.** The data was in the deployed build the whole time.

---

## 1. Map binding — RESOLVED, 99/99

### 1.1 The chain

```
SNATIVEID(wMainID, wSubID)          DB ChaSaveMap, SQL_C_LONG (s_COdbcGameChaGet.cpp:31, :213)
  -> mapslist.mst                   GLMapList::LoadMapsListFile   GLMapList.cpp:108
       file  = "GLMAPS_LIST", FileID = 0x0200
       body  = byte-substitution cipher, EMBYTECRYPT_MAPSLIST (ByteCrypt.cpp:46)
       DWORD dwNum, then SMAPNODE_DATA records (GLMapNode.cpp:133)
  -> Data\GLogic\Level\<strFile>    GLLevelFile::LoadFile          GLLevelFileSaveLoad.cpp:152
       cipher EMBYTECRYPT_LEVEL (ByteCrypt.cpp:28)
       SLEVEL_HEAD: m_strMapName, m_strWldFile (GLLevelHead.cpp:48-63)
  -> Data\Map\<m_strWldFile>        GLLandMan::LoadWldFile         GLLandManSet.cpp:19-20
  -> dwNAVI_MARK at 132+mark        SLAND_FILEMARK                DxLandDef.h:34-41
  -> NavigationCell[]               NavigationMesh::LoadFile      NavigationSaveLoad.cpp:51
```

`.lev` files sit in `Data\glogic\level\` — 144 of them. `SUBPATH::LEVEL_FILE_ROOT` is
`\Data\GLogic\Level\` (`SUBPATH.cpp:52`), and every `strFile` in `mapslist.mst` resolves
inside that directory.

### 1.2 Evidence

Decoded and parsed all three container formats with a probe that reimplements
`byte_decode`, `SMAPNODE_DATA::LOAD`, `SLEVEL_HEAD::LOAD_0102` and
`SLAND_FILEMARK::LoadSet` exactly as written. The mapslist probe consumed **11,362 of
11,362** body bytes with **99 records** and no residue, which is the check that matters: a
wrong field order or a wrong table would leave bytes over or run past the end.

```
mapslist records              : 99
resolved to an existing .wld  : 99
  of those, WITH a navmesh    : 99
  of those, without           : 0
```

Sample of the resolved table (all 99 in the report's probe output):

```
mapID  level file             wld file                 cells    vtx   map name
0/0    innerzone_01.Lev       innerzone_01.wld         4392   12523  SG_Campus1F
2/0    w_school_01.Lev        w_school_01.wld         13206   24893  SG_Campus
16/0   w_city_c_01.lev        1city_sportzone.wld     21810   20352  TradingHole
223/0  w_ground_zero_ch1.lev  ground_zero.wld         13273   39819  Lost City A
224/0  w_ground_lost_ch1.lev  Ground_Lost.wld          4777   14331  Chaos Island A
```

Two consequences worth stating because they change what a server must do:

1. **Case matters.** The deployed tree mixes `.Lev` and `.lev` (`w_school_01.Lev` vs
   `w_city_s_01.lev`), and RAN opens by concatenated path on Windows. A modern loader that
   lowercases before lookup is *more* permissive than legacy and will silently load a file
   legacy would not have found. Resolved case-insensitively here; the mismatch is recorded,
   not normalised away.
2. **Several map IDs share a `.wld`.** `w_city_s_02.lev` and `w_city_s_03.lev` both name
   `suhak.wld`; four club-war maps share `clubwar_inzone.wld`. So map ID → mesh is a
   *many-to-one* relation and the mesh must be cached and shared, not loaded per character.

### 1.3 `dwFieldSID`

Every one of the 99 records carries `dwFieldSID = 0`. RAN's `LoadMapsListFile` validates it
against `pServerInfo[...+dwFieldSID]` (`GLMapList.cpp:185`) and the deployed list declares
a single field server. A multi-field-server deployment is out of scope and is not modelled.

### 1.4 What is NOT in the chain

`m_MapID` inside the `.wld` is **0 for all 87 files** — verified by reading byte 132 of
every one. The `.wld` carries no map identity; it is addressed entirely through
`mapslist.mst`. So `GLLandMan::LoadWldFile`'s comment that "the `.wld` contains the map ID"
(`GLLandManSet.cpp:53`) describes an intent the shipped files do not carry, and a modern
loader must not expect a `.wld` to be self-identifying.

---

## 2. `default.charclass` — DECRYPTED, and it was never the speed source

### 2.1 The decryption

`CStringFile::Open` (`StringFile.cpp:84-99`) reads a leading `int` version and calls
`CRijndael::Initialize(version, sm_Version[version-1], sm_KeyLength[version-1],
sm_chain0, ...)`. The shipped file's version is **8**, so the key is
`sm_Version[7] = "lvdqkrmf$rpgo!@#$htjgj@#qksskrkr"`, 32 bytes, **AES-256**, and
`sm_chain0` is all zeros with `iMode = ECB` (`Rijndael.h:103`) — so no IV.

**002b missed a transform.** `Initialize` does not use the key string directly:

```cpp
// Rijndael.cpp:975-981
if( nVersion >= 0x0005 )
{
    for ( int i=0; i<keydatalength; i++ )
    {
        keydec[i] = keydec[i] ^ 0x21;
        keydec[i] += ( 0x21 ^ 0x10 );
    };
}
```

Without it, no `sm_Version` entry and no AES mode produces text — a sweep of all 8 keys ×
{128,256} bits × {ECB, CBC, CFB, OFB} × both body offsets topped out at **42.1 % printable**,
which is noise. With it, version 8 / AES-256 / ECB / body-after-4-bytes gives **92.4 %** with
longest identifier run `bodyeffect_distract.effskin_a`. That is the difference between "the
keys are in source" and "the keys are in source and transformed".

### 2.2 There is no `fWALKVELO` in `default.charclass`

This corrects 002b and 002a. `default.charclass` has **333 flags and none of them is
`fWALKVELO` or `fRUNVELO`**. Those keys are read by a *different* function,
`GLCONST_CHARCLASS::LOADFILE` (`GLogicDataLoad.cpp:1169-1170`), which is reached by a
per-class indirection:

```cpp
// GLogicDataLoad.cpp:557-570
for ( i=0; i < GLCI_NUM_8CLASS; i++ )
{
    if ( cFILE.findflag(szSETFILE[i]) )
    {
        cFILE.getflag( szSETFILE[i], 1, 1, strArg );
        BOOL bOk = cCONSTCLASS[i].LOADFILE ( strArg.GetString() );
```

`default.charclass` supplies all **16** `*.SETFILE` keys, one per class index, and each
names a `class<N>.classconst`. So the chain is two files, and anyone looking for the speeds
in `default.charclass` will not find them — which is exactly what 002a and 002b did.

### 2.3 The recovered base speeds

All 16, from `class0..classF.classconst`, decrypted with the same routine:

| `cCONSTCLASS` | SETFILE key | file | fWALKVELO | fRUNVELO |
|---:|---|---|---:|---:|
| 0 | `BRAWLER_M.SETFILE` | class0.classconst | 14.0 | 37.0 |
| 1 | `SWORDSMAN_M.SETFILE` | class1.classconst | 12.0 | 36.0 |
| 2 | `ARCHER_W.SETFILE` | class2.classconst | 16.0 | 42.0 |
| 3 | `SHAMAN_W.SETFILE` | class3.classconst | 13.0 | 40.0 |
| 4 | `EXTREME_M.SETFILE` | class4.classconst | 14.0 | 39.0 |
| 5 | `EXTREME_W.SETFILE` | class5.classconst | 14.0 | 39.0 |
| 6 | `BRAWLER_W.SETFILE` | class6.classconst | 14.0 | 37.0 |
| 7 | `SWORDSMAN_W.SETFILE` | class7.classconst | 12.0 | 36.0 |
| 8 | `ARCHER_M.SETFILE` | class8.classconst | 16.0 | 42.0 |
| 9 | `SHAMAN_M.SETFILE` | class9.classconst | 12.0 | 39.0 |
| 10 | `GUNNER_M.SETFILE` | classA.classconst | 15.0 | 40.0 |
| 11 | `GUNNER_W.SETFILE` | classB.classconst | 15.0 | 40.0 |
| 12 | `ASSASSIN_M.SETFILE` | classC.classconst | 12.0 | 44.0 |
| 13 | `ASSASSIN_W.SETFILE` | classD.classconst | 12.0 | 44.0 |
| 14 | `TRICKER_M.SETFILE` | classE.classconst | 15.0 | 41.0 |
| 15 | `TRICKER_W.SETFILE` | classF.classconst | 15.0 | 41.0 |

```
distinct fWALKVELO : 12.0 13.0 14.0 15.0 16.0
distinct fRUNVELO  : 36.0 37.0 39.0 40.0 41.0 42.0 44.0
```

The class index order is `EMCHARINDEX` (`GLCharDefine.h:237-252`), reached from the wire
class via `CharClassToIndex` (`GLCharDefine.cpp:91-118`), assigned at
`GLCharClient.cpp:251`. So `WorldCharacter::characterClass` — which is already carried and
already documented as "raw; not interpreted" (`WorldCharacter.h:125`) — is the exact input
this table is indexed by. The table is data, so it should be **loaded from the assets, not
compiled in**; the numbers above are the measured values a loader must reproduce.

The 002b fallback of 12/34 — the `GLCONST_CHARCLASS` constructor defaults
(`GLogicData.h:118-119`) — is **wrong for every class**. RAN overwrites both fields from the
`.classconst` files at startup, so those defaults never reach a running game.

---

## 3. The speed formula, in full

```cpp
// GLChar.cpp:4966-4974
float GLChar::GetMoveVelo ()
{
    float fDefaultVelo = IsSTATE(EM_ACT_RUN)
        ? cCONSTCLASS[m_CHARINDEX].fRUNVELO
        : cCONSTCLASS[m_CHARINDEX].fWALKVELO;
    return MoveVelocity( fDefaultVelo, GETMOVEVELO(), GETMOVE_ITEM(), IsSTATE(EM_ACT_RUN) );
}
```

Three terms, not two:

| Term | Formula | Source |
|---|---|---|
| base | `fRUNVELO` if `EM_ACT_RUN` else `fWALKVELO` | §2.3, per class |
| `GETMOVEVELO()` | `max(0, m_fSTATE_MOVE + m_fSKILL_MOVE + m_fOPTION_MOVE + m_sSUMITEM.fIncR_MoveSpeed)` | `GameCharacterCalculations.cpp:1109-1122` |
| `GETMOVE_ITEM()` | `max(0, m_sSUMITEM.fInc_MoveSpeed / cCONSTCLASS[m_CHARINDEX].fRUNVELO)` | `GLogixExPC.cpp:3059-3068` |

and

```
speed = base * (GETMOVEVELO() + GETMOVE_ITEM())      GameCharacterCalculations.cpp:294-306
```

Four things follow that matter for a first implementation:

1. **The divisor is `fRUNVELO` even when walking.** `GETMOVE_ITEM()` divides by run velocity
   unconditionally (`GLogixExPC.cpp:3065`), so a walking character's equipment bonus is
   scaled by a different base than its walk speed. Reproducing this exactly means the
   divisor is a per-class constant independent of the run/walk branch.
2. **`m_fSTATE_MOVE` is a multiplier that starts at 1.0, not a bonus.**
   `GLogixExPC.cpp:73` sets it to `1.0f`; state blows add to it (`:2500-2520`); death resets
   it to `0.0f` (`:2443`); it is clamped at `0.0f`. A character with no blows therefore has
   `GETMOVEVELO() = 1.0 + 0 + 0 + 0 = 1.0` and `speed = base × 1.0`. That is the *reason*
   RAN has no separate "speed bonus" constant for the base case — but it also means a modern
   implementation must not treat the term as additive-with-a-zero-default.
3. **`m_fOPTION_MOVE` is a pet/option ramp**, `+= 0.1f` clamped to `[0.0f, 3.0f]`
   (`GLogixExPC.cpp:272-283`). Out of scope for a milestone with no pets, but it is a
   *multiplier ceiling of 3.0*, not a bonus cap.
4. **`GetMoveVelo()` is re-evaluated every tick**, not once per GOTO:
   `GLChar.cpp:6089` inside `case GLAT_MOVE`, before `m_actorMove.Update`. So a speed change
   from a state blow takes effect on the next frame without a new 3034.

### 3.1 `IMovementSpeedProvider`

Per §21 this stays a seam, but the reason has changed materially. 002a's note said base speed
needed a decoder and equipment was unavailable. Now:

- **base speed: available.** Data recovered, chain proven, class index identified.
- **equipment: available as data, absent as state.** See §4.

So a `CharClassMovementSpeed` implementation is possible *today* for the base term, and the
provider interface should keep the equipment term separate rather than folding a zero into
it. What is still missing is not the numbers — it is the pipeline that turns the assets into
a table, which is implementation, not investigation.

---

## 4. Equipment movement speed — DATA EXISTS (classification **A** for the table, **C** for the worn set)

§12's four categories, answered precisely because "A" and "C" are both partly true and
collapsing them would hide the distinction.

**The modifier magnitude is in shipped data.** `item.csv` (22,284,068 B, 18,447 rows, 399
columns) is a CSV export of `SITEM` and carries `sVOLUME emTYPE` (column 128) and
`sVOLUME fVolume` (column 129). Reading `emTYPE == EMVAR_MOVE_SPEED` (`GLItemDef.h:543`,
value 5) yields:

```
items with sVOLUME emTYPE == EMVAR_MOVE_SPEED: 731
distinct fVolume values: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 20 21 22 23 24 26 30 60 90 100 150
  76/2    Supervisor Ring A          fVolume = 150.0
  128/0   Daestic Set Armor          fVolume = 60.0
  161/1   The Cloud Board            fVolume = 10.0
  162/4   38 pulse coil              fVolume = 1.0
```

The accumulation is `GLogixExPC.cpp:616-619`:

```cpp
case EMVAR_MOVE_SPEED:
    if( emSLOT != SLOT_VEHICLE ) m_sSUMITEM.fInc_MoveSpeed += sItemCustom.GETMOVESPEED();
```

over `SLOT_NSIZE_S_2 = 21` wearable slots, and `GETMOVESPEED()` (`GLItem.cpp:2746-2762`) is
`sVOLUME.fVolume` scaled by `GETOptVALUE(EMR_OPT_MOVE_SPEED)` as `(100 + rate) * 0.01`.

**What is missing is the worn set, not the data.** `WorldCharacter` carries no equipment —
deliberately, per `WorldCharacter.h:29` — and `modern/core/item/ItemDefinition.h`'s
`ItemStatBlock` has **no movement-speed field at all** (23 fields: six stats, three resources,
three powers, hit/avoid/defense, two damage, five resists, requiredSP). So a modern server
would have to *add* a field that does not exist, and populate it from a table it does not
have.

**Classification: A for the table, C for the runtime term.** The honest reading is that
equipment speed is *deferrable without falsifying movement* for a bounded scope — a character
with no equipment has `fInc_MoveSpeed = 0`, hence `GETMOVE_ITEM() = 0`, hence
`speed = base × GETMOVEVELO()`. That is not a fabricated zero; it is the exact value for the
state the modern server actually models. But it must be **documented as a deferral with a
named consequence** — RAN characters wearing a speed item move faster, and a modern one will
not — and not silently absorbed, per §30.

---

## 5. Obfuscated `.wld` — 14/14 decoded

`CWLDCrypt::Decryption_WLD` (`WLDCrypt.cpp:30-53`) with `bTool = true`: header becomes
`defaultMap`, and every byte past 131 is `b -= (K1 ^ K2); b ^= K1; b ^= K2` with
`K1 = 0x99701AE`, `K2 = 0x92617BE` (`WLDCrypt.h:18-19`). Truncating to `char` at each step
gives an effective low-byte key of `0x10` for the add/xor pair.

| file | orig | decoded | passes | FileID | navoff | bExist | cells | vtx | result |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---|
| Ground_Lost.wld | 1,278,660 | 1,278,660 | 1 | 0x0114 | 100154 | 1 | 4777 | 14331 | OK |
| LavaCity_re.wld | 591,536 | 591,536 | 1 | 0x0114 | 75934 | 1 | 2089 | 6267 | OK |
| Lead01.wld | 536,941 | 536,941 | 1 | 0x0114 | 74638 | 1 | 2022 | 1365 | OK |
| Lead01_waitingroom.wld | 54,196 | 54,196 | 1 | 0x0114 | 14814 | 1 | 158 | 474 | OK |
| deasindong_05.wld | 34,743,510 | 34,743,510 | 1 | 0x0114 | 31558908 | 1 | 12913 | 38739 | OK |
| **es_f41_killbillzone01.wld** | 272,774 | 272,774 | **2** | 0x0114 | 231168 | 1 | 168 | 504 | OK |
| es_top_laststage01.wld | 568,277 | 568,277 | 1 | 0x0114 | 362950 | 1 | 828 | 2484 | OK |
| event_farm.wld | 149,826 | 149,826 | 1 | 0x0114 | 5148 | 1 | 584 | 1752 | OK |
| fs_main01.wld | 3,182,372 | 3,182,372 | 1 | 0x0114 | 39292 | 1 | 14375 | 8842 | OK |
| ground_zero.wld | 3,510,794 | 3,510,794 | 1 | 0x0114 | 236504 | 1 | 13273 | 39819 | OK |
| rwpt2019.wld | 1,261,173 | 1,261,173 | 1 | 0x0114 | 129837 | 1 | 5152 | 3449 | OK |
| sps_ground.wld | 4,761,626 | 4,761,626 | 1 | 0x0114 | 61242 | 1 | 19021 | 57063 | OK |
| w_city03_new01.wld | 976,426 | 976,426 | 1 | 0x0114 | 328568 | 1 | 2637 | 7911 | OK |
| w_school_03.wld | 3,929,034 | 3,929,034 | 1 | 0x0114 | 232892 | 1 | 14970 | 44910 | OK |

**`es_f41_killbillzone01.wld` is encrypted twice.** One pass yields a valid `LAND.MAN` header
and a plausible `FileID` of `0x0114`, but the map-name field is 0x20 filler and the filemark
version reads `0x20202120` — four ASCII spaces. Two passes yield `MapID = 0` and a filemark
of `{ver 0x0100, size 16, navi 231036}` that parses to 168 clean cells. The distinguishing
signal is that the *header* is written by the `bTool` branch, which is idempotent, so a
correct-looking header proves nothing about the body.

Decoded output goes to a temp directory; originals untouched. Round-trip check:
`decrypt(original)` reproduces the stored decode byte-for-byte for **13/14**, and the
fourteenth is the double-encrypted file where the check must apply two passes — consistent,
not a failure. Note this validates the *decryptor against itself*; `Encryption_WLD` is a
different function and was not exercised, so "reversible" is asserted from the source
(`Decryption_WLD` is the algebraic inverse of `Encryption_WLD`) rather than measured.

---

## 6. Navigation data vs navigation engine — three separate answers

Per §17, not collapsed:

```
DATA AVAILABLE              = YES   77/87 .wld, 359,366 cells, 625,216 vertices, 0 anomalies
NAVIGATION QUERY IMPLEMENTABLE = YES   pure geometry over the parsed cells
MOVEMENT UPDATE IMPLEMENTABLE   = YES   given speed (§2/§3) and a tick source (§9)
```

### 6.1 Full `.wld` inventory

```
total .wld            : 87
  obfuscated          : 14   (all 14 decoded)
  plain               : 73
navmesh parsed OK     : 77
  cells total         : 359366
  vertices total      : 625216
  anomalies           : badID=0 badPlane=0 badLink=0
no navmesh (bExist=0) : 9    character1_slt, character_slt, character_slt_main,
                              character_slt_old, character_slt_s01..s03,
                              log_in, login
unsupported FileID    : 1    square_rd.wld 0x0202
```

The 9 without a mesh are character-select and login maps, which RAN never walks — consistent
with 002b's reading, now with the cause identified (`bExist` is literally `0` at
`dwNAVI_MARK`). `square_rd.wld` carries `FileID 0x0202`, which
`NSLANDMAN_SUPPORT::IsLandManSupported` does not list, so **legacy itself would refuse to
load it**. Not a modern limitation.

**Every one of the 99 registered maps has a navigation mesh**, so the 10 unreachable files
are all unregistered content.

### 6.2 Format, confirmed unchanged from 002b

`NavigationCell` = **188 bytes**; `Line2D` = 28 (three `D3DXVECTOR2` + `bool` + padding, not
the 20 sometimes quoted); `Plane` = 28. The three consistency checks 002b established still
hold across all 77 meshes: `CellID == array index` (the invariant
`NavigationMesh::GetCell` silently depends on, `navigationmesh.h:140-144`), `|normal.y| ≤ 1`,
and every `LinkID` in range.

One correction to a detail in the 002b report: the navigation block is **not** at
`132 + dwNAVI_MARK`. It is at `132 + dwNAVI_MARK + 4`, because `GLLandMan::LoadWldFile`
reads a `DWORD bExist` at the mark first (`GLLandManSet.cpp:65-67`). Parsing at the mark
itself yields garbage cell counts on every file; the 4-byte shift is what makes all 77 parse.

---

## 7. Minimum navigation kernel

### 7.1 Dependency graph from `Actor::Update`

```
Actor::Update                       actor.cpp:302      REQUIRED
  ├─ D3DXVec3Length/Normalize       math               REPLACEABLE (Vector3 has both)
  ├─ NavigationMesh::ResolveMotionOnMesh              REQUIRED
  │    ├─ NavigationCell::ClassifyPathToCell          REQUIRED
  │    │    └─ Line2D (SignedDistance/ClassifyPoint/Intersection)
  │    ├─ NavigationCell::ProjectPathOnCellWall       REQUIRED  (slide, *0.98f)
  │    ├─ NavigationCell::ForcePointToCellCollumn     REQUIRED  (NO_RELATIONSHIP)
  │    ├─ NavigationCell::MapVectorHeightToCell        REQUIRED  (Y from Plane)
  │    ├─ NavigationMesh::GetCell                     REQUIRED
  │    └─ NavigationCell::Normal()                    REQUIRED  (Y suppression)
  ├─ NavigationMesh::GetCell                         REQUIRED
  └─ NavigationPath::GetFurthestVisibleWayPoint       REQUIRED

Actor::GotoLocation(vp1, vp2)       actor.cpp:476      REQUIRED  (the ±10 probe)
  └─ NavigationMesh::IsCollision                     REQUIRED
       ├─ DxAABBNode::IsCollision                     REQUIRED
       └─ COLLISION::IsLineTriangleCollision          REQUIRED

Actor::GotoLocation(pos, cellID)    actor.cpp:418      REQUIRED  (path build)
  ├─ NavigationMesh::LineOfSightTest                 REQUIRED  (2-waypoint fast path)
  ├─ NavigationMesh::FindClosestCell                 REQUIRED
  ├─ NavigationCell::IsPointInCellCollumn            REQUIRED
  └─ NavigationMesh::BuildNavigationPath             REQUIRED  (A*)
       ├─ NavigationHeap                              REQUIRED
       └─ NavigationCell::ProcessCell/QueryForPath/ComputeHeuristic  REQUIRED

Actor::Create / SetPosition         actor.cpp:58/106   REQUIRED  (spawn + snap)
  ├─ NavigationMesh::IsCollision                     REQUIRED
  ├─ NavigationMesh::FindClosestCell                 REQUIRED
  └─ NavigationMesh::SnapPointToCell                 REQUIRED

NavigationMesh::MakeAABBTree        navagationtree.cpp:489  REQUIRED  (called by LoadFile)
  ├─ GetSizeNode / GetCenterDistNode / MakeAABBNode   REQUIRED
  └─ IsWithInTriangle                                 REQUIRED

NavigationMesh::LoadFile            NavigationSaveLoad.cpp:51  REQUIRED
NavigationCell::LoadFile            NavigationSaveLoad.cpp:121 REQUIRED
```

### 7.2 Size

| Unit | lines | file |
|---|---:|---|
| `Actor` (Create, SetPosition, Update, GotoLocation ×2, GetTargetPosition) | 196 | actor.cpp |
| `NavigationMesh` (SnapPointToCell, FindClosestCell, BuildNavigationPath, ResolveMotionOnMesh, LineOfSightTest, GetCell) | 312 | navigationmesh.cpp |
| `NavigationCell` (Classify, ProjectWall, ForceWall, ForceCollumn, A* ×3) | 349 | navigationcell.cpp |
| AABB tree (GetSize, GetCenterDist, IsWithInTriangle, MakeNode, IsCollision ×2, MakeTree) | 420 | navagationtree.cpp |
| `Line2D` | 170 | line2d.h |
| `Plane` | 100 | plane.h |
| `NavigationHeap` | 180 | navigationheap.h |
| `NavigationPath` | 110 | navigationpath.h |
| `LoadFile` ×2 | 118 | NavigationSaveLoad.cpp |
| `COLLISION::IsLineTriangleCollision` + `DxAABBNode::IsCollision` | 281 | Collision.cpp |
| **total** | **2,236** | |

Excluded, with reasons: `Actor::Render` 133 and `NavigationMesh::Render` ~120 and
`CreateVBIB`/`DeleteVBIB` ~80 are debug drawing; `playpen.cpp` 852 is a standalone sample
with no callers; `Navigations.cpp/h` 46 is a namespace stub; `CheckIntegrity` and
`GotoErrorPosition` ~130 are editor-time diagnostics.

So 002b's "~2,890 lines" was close, and its conclusion that this is "too large" was a
judgement about *effort*, not about *boundedness*. It is bounded: a fixed, enumerated
function list, no recursion into the map engine, no rendering.

### 7.3 D3DX9 dependency — smaller than 002b reported

Per-file D3DX mention counts across `legacy/Lib_Engine/NaviMesh`:

```
actor.cpp               440 lines   41 mentions
navigationmesh.cpp      896 lines   54
navagationtree.cpp      453 lines   51
navigationcell.h        362 lines   37
line2d.h                295 lines   28
navigationmesh.h        119 lines   22
actor.h                 136 lines   21
navigationcell.cpp      383 lines   17
plane.h                 168 lines   15
NavigationSaveLoad.cpp  118 lines   10
navigationheap.h        215 lines    5
navigationpath.h        121 lines    5
playpen.cpp             852 lines  382   (excluded)
```

The important point is *what kind* of D3DX it is. In the required set:

- `D3DXVECTOR3` (625 uses repo-wide) — a 3-float struct. `Modern::Vector3`
  (`modern/core/math/Vector3.h`) already has `+ - * / += -= *= /=`, `Length`, `LengthSq`,
  `Distance`, `IsFinite`, `Normalize`. Missing: `Dot`, `Cross`, `operator[]`. Three small
  additions.
- `D3DXVECTOR2` (61 uses) — same, minus Y. Needs a new `Vector2`, ~20 lines, for `Line2D`.
- `D3DXMATRIX` (13 uses) — **only ever an identity matrix.** `MakeAABBTree` builds one with
  `D3DXMatrixIdentity` (`navagationtree.cpp:503`) and threads it through
  `D3DXVec3TransformCoord` unchanged. `D3DXVec3TransformCoord` with identity is the
  identity function, so all 13 uses collapse to a copy. **No matrix type needed at all.**
- `D3DXVec3Length/Normalize/Dot/Cross/TransformCoord`, `D3DXVec2Dot/Normalize` — 6
  transcendental/algebraic helpers, each a direct expression in the new vector types.
- `D3DXMESH`, `D3DXMESH_32BIT`, `D3DXGetFVFVertexSize`, `LPDIRECT3DDEVICEQ` — **all in
  `CreateVBIB`/`DeleteVBIB`/`Render`, which are excluded.**

The two genuinely non-obvious include dependencies are `navigationmesh.cpp:5-9`
(`DxCommon/collision.h`, `DxViewPort.h`, `DxOctree/DxLandMan.h`, `DxLightMan.h`) and
`navagationtree.cpp:3`. Resolved by reading what the *required* functions actually call:

- `DxLandMan.h`, `DxLightMan.h`, `DxViewPort.h` — **used only by excluded functions.**
  `DxViewPort::CameraJump` is at `navigationmesh.cpp:623`, inside `GotoErrorPosition`.
  `ExportProgress::CurPos++` at `:453` and `:647` is inside `LinkCells` and `AddCell`, both
  editor-time; `LinkCells` is *not* in the required set because `LoadFile` restores links
  from the file (`NavigationSaveLoad.cpp:76-96`) rather than recomputing them.
- `DxCommon/collision.h` — **genuinely required**, for `DxAABBNode` and the two collision
  functions. But 222 lines of header for **two** functions and a **19-field POD node** is
  not a dependency on the collision *system*; it is a dependency on ~281 lines of arithmetic
  in `Collision.cpp` that happen to live behind a wide header.

**This is the real answer to 002b's D3DX blocker: there is no rendering dependency, no
resource dependency, and no terrain dependency in the required set. There is a maths
dependency, and the modern tree already owns 90 % of the maths type it needs.**

### 7.4 The `playpen.cpp` note

`playpen.cpp` is 852 lines of hard-coded sample geometry (381 points, 456 polys) with no
callers. It is the reason a naive grep makes the subsystem look enormous, and it should not
be counted in any estimate.

---

## 8. GOTO validation, in full

`GLChar::MsgGoto` (`GLCharMsg.cpp:224-324`), in order:

| # | Check | Action on failure | Line |
|---|---|---|---|
| 1 | `m_bEmptyMsg` | drop | 226 |
| 2 | `m_sPMarket.IsOpen()` | `E_FAIL` | 227 |
| 3 | `m_bSTATE_STUN` | `Stop()`, `E_FAIL` | 230-235 |
| 4 | `IsSTATE(EM_ACT_DIE)` | send `GLAT_FALLING` to self, `E_FAIL` | 238-250 |
| 5 | **`EM_ACT_RUN` from the packet** | applied to the persistent word | 254-262 |
| 6 | **`\|m_vPos − vCurPos\| > 60.0f`** | if TALK/GATHERING: `E_FAIL`. else `TurnAction(GLAT_IDLE)`, send `SNET_GM_MOVE2GATE_FB` (3830) to self, broadcast `SNETPC_JUMP_POS_BRD` (3064) to view, `S_OK` | 264-288 |
| 7 | **destination accepted** | stored verbatim: `m_TargetID.vPos = pNetMsg->vTarPos` | 290 |
| 8 | `TurnAction(GLAT_MOVE)` | — | 291 |
| 9 | **±10 vertical probe** | `GotoLocation(tar+10y, tar−10y)`; `bSucceed == FALSE` ⇒ **no path, no 3035**, but `MsgSendUpdateState` still runs | 293-302 |
| 10 | `SetMaxSpeed(GetMoveVelo())` | only if step 9 succeeded | 306-307 |
| 11 | **3035 broadcast** | only if step 9 succeeded | 311-318 |
| 12 | `MsgSendUpdateState(false,false,true)` | always | 321 |

Three things this settles that 002b left open:

**The destination is genuinely unvalidated at the protocol layer, and the navmesh is the
only gate.** Step 7 stores whatever arrived. Step 9 is the entire destination check, and it
is a *vertical* one: a 20-unit vertical segment at the destination's XZ, tested against the
mesh's triangles. A destination outside the mesh's XZ extent fails it — but a destination
inside the extent at any Y within ±10 succeeds regardless of XZ walls. Walls are handled
later, per step, by `ResolveMotionOnMesh`. Verified from data that this probe has geometry to
hit on all 77 meshes: the ±10 band around each mesh's own centre-Y range intersects
**100 %** of triangles in **77/77**.

**A failed probe is silent.** `bSucceed == FALSE` produces no 3035, no error, and no
position change. The client learns nothing. That is RAN's behaviour and it is observable.

**The 60-unit snap-back uses the client's claimed `vCurPos`, not the destination.** It is a
desynchronisation check, and it is the only place the packet's `vCurPos` is read at all
(`:264`). It also has the side effect of applying the packet's `EM_ACT_RUN` bit *before*
deciding to abort (`:258-262` precede `:264-288`) — so a desynced client can still change the
server's run flag. That ordering is a real quirk and must be reproduced, not tidied.

---

## 9. Movement algorithm and timing

Complete statement, so no part of §24 is left to the implementer.

```
per server tick, for each GLChar with IsSTATE(EM_GETVA_AFTER) and m_Action == GLAT_MOVE:

  if (m_bSTATE_STUN && PathIsActive()) Stop();

  m_actorMove.SetMaxSpeed(GetMoveVelo());          GLChar.cpp:6089
  m_actorMove.Update(fElapsedTime);                 GLChar.cpp:6090

      // Actor::Update, actor.cpp:302
      if (!m_Parent)      return E_FAIL;
      if (!m_PathActive)  return S_OK;

      m_fMovedTime += elapsedTime;

      m_Movement = nextWaypoint.Position - m_Position;      // or zero + m_PathActive=false
      max_distance = m_MaxSpeed * elapsedTime;              // :327
      distance     = |m_Movement|;
      if (distance > max_distance) { Normalize(m_Movement); m_Movement *= max_distance; }

      if (distance > 0.01f) {                                // :347  ARRIVAL THRESHOLD
          NextPosition = m_Position + m_Movement;
          ResolveMotionOnMesh(m_Position, m_CurrentCellID, NextPosition, &NextCellID);
          NextCell = GetCell(NextCellID);
          if (!NextCell) return E_FAIL;
          if (NextCell->Normal().y <= 0.0001f) NextPosition.y = m_CorrectY;   // :361-363
          else                               m_CorrectY  = NextPosition.y;
          m_Position = NextPosition;
          m_CurrentCellID = NextCellID;
      } else {
          m_Position = nextWaypoint.Position;               // :375  SNAP TO WAYPOINT
          if (pCell->Normal().y <= 0.0001f) m_Position.y = m_CorrectY;
          else                               m_CorrectY  = m_Position.y;
          distance = 0; m_Movement = 0;
          m_NextWaypoint = GetFurthestVisibleWayPoint(m_NextWaypoint);
          if (end) { m_PathActive = false; m_NextPosition = FLT_MAX; }
          else     { m_NextPosition = next.Position; Update(elapsedTime); }  // :405 RECURSE
      }

  if (!PathIsActive()) { Stop(); TurnAction(GLAT_IDLE); }
  m_vPos = m_actorMove.Position();                          // :6099  THE ONLY WRITE

  // facing, derived — never on the wire
  if (NextPosition != FLT_MAX) {
      vDirection = NextPosition - m_vPos;
      if (!DxIsMinVector(vDirection, 0.2f)) { Normalize(vDirection); m_vDir = vDirection; }
  }
```

Answers to §24 and §25:

| Question | Answer | Evidence |
|---|---|---|
| direction | waypoint − position, in 3D; only XZ survive the mesh walk | actor.cpp:315-316 |
| distance | `\|movement\|`, **3D** — note `vMoveDist` at :337 is computed and discarded | actor.cpp:328 |
| step clamp | `max_distance = MaxSpeed × elapsedTime`; if longer, normalise and scale | actor.cpp:327-333 |
| Y suppression | components `\|v\| < 0.001f` zeroed; **a pure-vertical step is frozen** | actor.cpp:342-344 |
| vertical probe | destination ±10 only, once, at GOTO time | GLCharMsg.cpp:293-297 |
| collision query | `ResolveMotionOnMesh`, per step, against the cell graph — not raycasting | navigationmesh.cpp:241 |
| slide | `ProjectPathOnCellWall`, then `Direction *= 0.98f`. Comment says 10 %, code is 2 % | navigationmesh.cpp:302-308 |
| partial completion | yes — the step is clamped at the wall intersection | navigationmesh.cpp:295, 301 |
| position update | `m_Position = NextPosition` inside Update, then `m_vPos = Position()` outside | actor.cpp:370, GLChar.cpp:6099 |
| arrival | `distance <= 0.01f` ⇒ snap to waypoint exactly, then advance the waypoint | actor.cpp:347, 375 |
| destination clearing | `m_PathActive = false`; signal is `m_NextPosition == FLT_MAX` | actor.cpp:321, 400 |
| elapsed time | `timeGetTime() * 0.001`, real wall clock | DxServerInstance.cpp:290, 303 |
| tick rate | **not fixed.** `0.020f` is compared at :294 but `return S_FALSE` is commented out at :297, so it only `Sleep(0)`s | DxServerInstance.cpp:294-298 |
| frame pacing | 10 ms, and only when `use_event_thread` is on — **default off** | s_CCfg.cpp:72 |

**No fixed 60 Hz assumption is warranted, and none is needed.** The invariant that matters is
`Σ(MaxSpeed × Δt) ≤ MaxSpeed × wall_seconds`, which holds at any packet or frame rate because
`Δt` is measured. A modern server may use a deterministic tick *abstraction*; what it must
not do is convert elapsed time into a fixed per-tick constant, because that would let a
client's movement rate depend on the server's frame rate.

---

## 10. Broadcast — unchanged from 002b, with the model question settled

**3035 `SNETPC_GOTO_BRD`, 44 bytes, sent once per accepted GOTO.** `GLCharMsg.cpp:311-318`,
after the speed is set and only if the vertical probe succeeded.

**RAN transmits no authoritative position after movement advances.** `GLChar::FrameMove`
(`GLChar.cpp:5238-6198`) emits no position message; its broadcasts are brightness,
quest-fact-end and PK-combo-end. The only position corrections are event-driven
`SNETPC_JUMP_POS_BRD` (3064, the desync snap-back at `:282-285`) and
`SNET_GM_MOVE2GATE_FB` (3830, at `:276-278`).

The client reads 3035 at `GLCharClient.cpp:1720-1742` and uses **only `dwActState` and
`vTarPos`** — `vCurPos` and `fDelay` are decoded and discarded. It then runs the *same*
`Actor` code on its own copy of the same mesh.

---

## 11. Movement model — **MODEL A**

Per §18/§19. MODEL A, on the evidence:

**MODEL A is implementable in the modern client.** The client has a deterministic frame loop
(`Application::Run` with `SetUpdateCallback`, `Application.cpp:109-165`), an input system
delivering `MouseMove`/`MouseButton` per frame (`InputEvents.h:76-113`), a
`ClientCharacterState` with `GetPosition()` (`ClientCharacterState.h:74`), a
`WorldEntryClient` that already builds and feeds movement messages
(`WorldEntryClient.cpp:122, 352`), and a `Vector3` with the arithmetic prediction needs. It
has **no** mesh, no movement controller and no prediction loop — those are new — but nothing
in the architecture forbids them, and none of it requires the legacy renderer.

**MODEL B would be a new protocol, not a modernization.** §31 requires a separate
specification; it is not proposed here.

**The honest cost of MODEL A** is that it duplicates the navigation kernel client-side. That
is what RAN does, and the alternative is a protocol RAN never had. Given that the current
phase's stated purpose is connecting modern code to the proven RAN protocol, introducing an
invented position stream now would be the larger deviation.

One consequence to record: under MODEL A the server's authoritative position is **never
transmitted**, so a modern client cannot reconcile. It is also never *needed* for
correctness of the mover's own experience — only for cross-client agreement, which RAN also
does not provide and instead papers over with the 3064 event. A modern implementation that
wants reconciliation needs MODEL B, on the record, as a documented deviation.

---

## 12. Modern feasibility

| | |
|---|---|
| map → `.wld` | **PROVEN** — `mapslist.mst` → `.lev` → `.wld`, 99/99 |
| navigation data | **AVAILABLE** — 77 meshes, 359,366 cells, 0 anomalies |
| obfuscated `.wld` | **DECODED** — 14/14 |
| base speed | **RECOVERED** — 16 classes, measured |
| speed formula | **UNDERSTOOD** — three terms, exact |
| equipment speed | **DATA AVAILABLE**, runtime term absent — deferrable with a documented consequence |
| navmesh kernel | **BOUNDED** — 2,236 logical lines, enumerated |
| D3DX9 | **BOUNDED** — maths only; no device, no terrain, no resources; `D3DXMATRIX` collapses to identity |
| GOTO validation | **UNDERSTOOD** — 12 steps, ordered |
| movement algorithm | **UNDERSTOOD** — complete, line by line |
| timing | **UNDERSTOOD** — measured `Δt`, no fixed rate |
| broadcast | **UNDERSTOOD** — 3035 once, no position stream |
| modern client | **COMPATIBLE** — loop, input, state, codec all present; mesh + predictor are new |

**Classification: A.** Data exists and format is understood; every dependency is either
proven available or explicitly deferrable with a stated consequence. The §29 gate is
satisfied on all twelve items.

---

## 13. Implementation readiness

**READY.** All twelve §29 gate items answered.

### 13.1 Proposed bounded scope for the next implementation phase

Sequenced so each step is independently verifiable, and each step before the last is useful
without it.

1. **Asset readers, no gameplay.** `modern/core/data`: mapslist, `.lev` (`SLEVEL_HEAD`),
   `.wld` header + `SLAND_FILEMARK` + `NavigationCell[]`, `class<N>.charconst` +
   `default.charclass`. Each with a round-trip/consistency test using the §1.1 and §6.2
   checks — the `CellID == index`, `|normal.y| ≤ 1`, and link-range checks, which are what
   make a wrong stride fail loudly instead of subtly.
2. **Maths types.** Add `Dot`/`Cross`/`operator[]` to `Vector3`; add `Vector2`. No matrix
   type. Note in `Vector3.h` *why* there is none.
3. **Navigation kernel, headless.** `Line2D`, `Plane`, `NavigationCell`, `NavigationMesh`
   (query + A* + `ResolveMotionOnMesh`), `AabbTree`, `IsLineTriangleCollision`,
   `DxAabbNode`. No rendering, no `DxLandMan`, no device. Verify against the §14 fixture
   points.
4. **`Actor` equivalent.** `Create`/`SetPosition`/`GotoLocation`×2/`Update`/`GetTargetPosition`,
   transliterated from §9 with the legacy constants (`0.01f`, `0.001f`, `0.0001f`,
   `0.98f`, `FLT_MAX` sentinel) named and commented as legacy constants.
5. **Speed.** `CharClassMovementSpeed` implementing `IMovementSpeedProvider`, **base term
   only**, with the equipment term named as deferred in the header and in the report. Data
   loaded from `class<N>.classconst`, not compiled in.
6. **Server-side destination + position.** Fields on `WorldCharacter` for authoritative
   position and destination, in the character record rather than a parallel structure, per
   §21. Update driven by an injected elapsed-time value, not an internal clock.
7. **GOTO codec + handler.** 3034 decode, the §8 validation chain, 3035 encode. Identity
   from the connection's authorized character — 3034 carries **no** id, exactly as 3032 does,
   so the existing `FieldSession` guarantee extends with no new attack surface.
8. **World tick.** A `FieldRoleRuntime` update loop feeding elapsed time to step 6. Must not
   assume a fixed rate (§9).

### 13.2 Explicitly out of scope

- No new network message. MODEL A.
- No rendering, no terrain, no `DxLandMan`.
- No client-side prediction (a separate milestone; the server half does not need it).
- No equipment speed, vehicle speed, disguise-combine speed, state blows, passive skills, or
  `m_fOPTION_MOVE` ramps. Each is a **named deferral**, and `GetMoveVelo` must be written so
  each can be added without touching the others.
- No sector/view-range system. As 002b found, the ±250 XZ square is a few lines; GOTO
  broadcast scope can reuse 002a's broadcast-to-all staging and be narrowed later.

### 13.3 Risk register

| Risk | Why it matters | Mitigation |
|---|---|---|
| cell stride regression | 164 vs 188 fails *quietly* on some files | assert `CellID == index` on load; 002b showed 164 yields 423 bad IDs on one file |
| `GetCell` indexing by position | `GetCell(LinkID)` indexes by array position, not `CellID` — a reordered load would silently mislink | keep the load order invariant and assert it |
| elapsed-time semantics | converting `Δt` to a fixed tick changes movement rate | inject `Δt`; test with two different `Δt` splits over the same interval and assert equal distance |
| speed defaults leaking in | 12/34 constructor defaults are wrong for every class; a test that forgets to load `.classconst` passes against them | test the loaded table against the measured 16 rows, not against the defaults |
| double-encrypted `.wld` | one shipped file needs two passes; a single-pass loader fails on it with a plausible-looking header | decode loop that retries while the filemark version is implausible, and record the pass count |

---

## 14. Test fixtures — real coordinates, derived from parsed data

Per §23. From `w_school_01.wld` (map 2/0, 13,206 cells, 24,893 vertices, **one** connected
component — verified by BFS over the link graph, so every cell is reachable):

| Case | Value | How derived |
|---|---|---|
| start | cell 6603, centre **(1031.327, −59.215, 997.280)** | cell centre read from the mesh |
| valid nearby destination | cell 6604, centre **(1024.451, −59.604, 1000.140)**, XZ distance **7.446** | link[0] of the start cell |
| multi-cell destination | cell 0, centre **(−51.226, 14.999, −2403.015)**, XZ distance **3568.463** | same component, non-adjacent ⇒ requires A* |
| obstructed | **(1057.718, −59.410, 1002.816)** | 20 units outward along the wall-1 midpoint (1038.144, −59.410, 998.710); side 1 has **no link**, so the step must clamp there and apply `× 0.98` |
| off-mesh | **(2460.000, −59.215, 3374.930)** | mesh XZ extent is [−2013.497, 1460.000] × [−2438.650, 2374.930] |
| vertical in-probe | Y ∈ **[−69.215, −49.215]** | centre ±10 |
| vertical out-of-probe | Y = **−48.215 … −9.215** | beyond +10 |

`w_school_01.wld` has 853 distinct cell-centre Y values spanning [−179.957, 15.095], so the
vertical cases are chosen against real terrain rather than a flat assumption.

A second set from `innerzone_01.wld` (map 0/0, 4,392 cells): start cell 2196 at
(353.062, 0.018, −253.750), linked neighbour 2195 at (354.365, 0.018, −266.357), unlinked
side 2, obstructed destination (354.746, 0.018, −221.169).

**These are measurements, not guesses.** Every coordinate is a cell centre or a
midpoint-plus-offset computed from the parsed navigation block.

---

## 15. Probes used

Investigation-only, run from a temp directory, not added to the repository.

| Probe | Purpose | Result |
|---|---|---|
| `mapslist_probe.py` | decode `mapslist.mst`, walk `SMAPNODE_DATA::LOAD` | 99 records, 11362/11362 bytes consumed, 0 residue |
| `lev_probe.py` | decode all 144 `.lev`, extract `m_strWldFile` | 144/144, every one names a `.wld` that exists |
| `mapbind_probe.py` | join mapslist → `.lev` → `.wld`, report gaps | 99/99 resolved, 0 missing |
| `map_chain.py` | full chain including navigation cell count per map | 99/99 with a navmesh |
| `wld_inventory.py` | decode + parse all 87 `.wld`, consistency checks | 77 OK, 0 anomalies, 9 no-mesh, 1 unsupported FileID |
| `wld_nav_probe.py` | locate `dwNAVI_MARK`, measure the 4-byte `bExist` shift | shift confirmed; pre-shift parse yields garbage counts |
| `wld_diag.py` | per-file diagnosis of parse failures | 9 × `bExist == 0`, 1 × unsupported FileID, 1 × double-encrypted |
| `wld_crypt_report.py` | decode the 14 obfuscated files, per-file table | 14/14, 92,967 cells, 0 anomalies; originals untouched |
| `nav_points_probe.py` | derive real start/destination/obstructed/off-mesh/vertical points | §14 fixtures |
| `vertical_probe.py` | does the ±10 probe have geometry to hit? | 100 % of triangles in 77/77 meshes |
| `charclass_read.py` | decrypt + parse `default.charclass` | 333 flags, **no `fWALKVELO`**, 16 `*.SETFILE` keys |
| `aes_modes.py` | find the decryption mode | the `nVersion >= 5` key transform is required; 42.1 % → 92.4 % printable |
| `classconst_read.py` | follow `*.SETFILE` and read the speeds | 16/16 classes, 5 distinct walk and 7 distinct run values |
| `dump_plain.py` | print a decrypted config verbatim, escaped | confirmed the `KEY value // comment` record syntax |
| `item_speed_probe.py` | enumerate `EMVAR_MOVE_SPEED` items from `item.csv` | 731 items, 29 distinct `fVolume` values |
| line/function accounting over `legacy/Lib_Engine/NaviMesh` | size the minimum kernel by exact line ranges | 2,236 lines; D3DX matrix use collapses to identity |

Two methodological notes, because both changed a conclusion:

- The **key-transform omission** (§2.1) is what made 002b report the decoder as blocked on
  "a legacy PCH dependency chain". It was never a PCH problem — the shipped file decrypts
  with 20 lines of Python. The negative result was a wrong assumption about the cipher, not a
  build obstacle.
- The **4-byte `bExist` shift** (§6.2) is why a first-pass `.wld` parser reports implausible
  cell counts on *every* file. Reading at `dwNAVI_MARK` instead of after the `bExist` DWORD
  looks like a format failure and is not one.

---

## 16. Regression status

Unchanged from `73466eb`:

```
ModernNetworkTests         193/193
ModernServerTests          228/228
ModernWorldEntryTcpTests    14/14
CTest Debug                 18/18
CTest Release               18/18
```

No 002a behaviour was modified. The per-connection LZO codec, the synchronised repository,
the atomic `FieldEntryRegistry::Claim` and the Field role's concurrent connections are all
untouched.
