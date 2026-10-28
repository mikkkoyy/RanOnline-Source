# MODERN-ASURA-CHARACTER-EQUIPMENT-003 — Runtime Evidence

## Screenshot

- **File:** `MODERN-ASURA-CHARACTER-EQUIPMENT-003.png`
- **Dimensions:** 1366 x 768
- **Source capture:** `C:\temp\equipment_screenshot_v2.png` (captured via `Graphics.CopyFromScreen`)
- **SHA-256:** `3D4D17142E886213D787598E73A5CFEEAD90AC22EEC95AE32F6865822576219A`

## Runtime Command

```
ModernClientApp.exe --field bambooforest --character data/skin/s_m_hb.X --head data/skin/s_m1_head.x
```

- **Asset root:** `D:\FILES\project\RanOnline-Build\ASURA CLIENT`
- **Field:** `bambooforest.wld0`

## Reported Runtime Behavior

Console output from the running client:

- Field `bambooforest`: 26 meshes, 78646 vertices, 26 textures uploaded — 0 failures
- Character body `s_m_hb.X`: skinned, 840 vertices, 52 bindings, 210 animation frames — GPU resource 1
- Character head `s_m1_head.x`: skinned, bound to `Bip01_Head`, 9 bindings, 12 sections — GPU resource 2
- Equipment block: item.csv exists, reader init OK, Extract OK (4 rows matched, 0 missing)
- **4 equipment pieces loaded:**
  - `CUN0001_m.cps` (type 1, upper) — 5 sections, 52 bindings — skinned, uploaded
  - `CDN0001_m.cps` (type 2, lower) — 5 sections, 52 bindings — skinned, uploaded
  - `CFN0034_m.cps` (type 6, foot) — 5 sections, 52 bindings — skinned, uploaded
  - `BTN0011_m.cps` (type 4, weapon) — 17 sections — rigid fallback attached to `Bip01_R_Hand` (frame index 71)
- Equipment textures resolved and uploaded: `m1_hb_body.dds`, `m2_bs_body.DDS`, `m1_khb_foot.dds`, `m1_hb_leg.dds`, `m1_hb_hand.dds`
- Character spawned at (-100.74, 17.51, -785.90)
- Animation ready: idle (1.033s), walk (1.533s)
- App ran in render loop with no crashes; clean shutdown

## Test Results

| Test Suite | Passed | Failed | Skipped | Total |
|---|---|---|---|---|
| ModernClientAsuraAssetPipelineTests | 10 | 0 | 0 | 10 |
| ModernClientCharacterAssetTests | 35 | 0 | 0 | 35 |
| ModernClientSkinnedMeshTests | 7 | 0 | 0 | 7 |
| ModernClientCharacterRigTests | 4 | 0 | 0 | 4 |
| ModernClientRanAnimationTests | 10 | 0 | 0 | 10 |
| **Total** | **66** | **0** | **0** | **66** |

All tests pass with `RAN_ASSET_ROOT=D:\FILES\project\RanOnline-Build\ASURA CLIENT`.

## Limitation

The image has **not yet been visually reviewed**. Console output confirms all equipment pieces loaded, textured, and had draw commands issued without errors, but visual confirmation of correct placement (armor alignment, weapon attachment position, no floating/stretching/clipping) cannot be asserted from this capture alone. Runtime logs are not a substitute for visual inspection.
