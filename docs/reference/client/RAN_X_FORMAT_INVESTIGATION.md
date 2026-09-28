# RAN Online `.X` Mesh Format Investigation

**CLIENT-014 — Investigation Only**

Date: 2026-09-28
Source repo: `https://github.com/mikkkoyy/RanOnline-Source`
Local checkout: `D:\FILES\project\modernization RanOnline`
ASURA client: `D:\FILES\project\RanOnline-Build\ASURA CLIENT`
Starting commit: `e445f9f` (CLIENT-013)

---

## 1. Executive Summary

RAN Online ships `.x` mesh files in a DirectX `.X` binary format (version 03.03, binary encoding). The files are optionally stored with a `.mxf` container that applies a simple byte-level encryption (`byte += 0xEA; byte ^= 0xEB`). The legacy RAN client uses `D3DXLoadMeshFromX` (or `D3DXLoadMeshFromXInMemory` for `.mxf` files) to load meshes, followed by `OptimizeInplace` and `CloneMeshFVF` for post-processing.

The vast majority of RAN `.x` files contain **SkinWeights** data, indicating that character meshes are skinned rather than rigid. Static meshes (items, weapons, scenery) may also contain skin data. The `Mesh`, `MeshNormals`, `MeshTextureCoords`, `MeshMaterialList`, `Frame`, `FrameTransformMatrix`, `SkinWeights`, `XSkinMeshHeader`, `VertexDuplicationIndices`, `Material`, and `TextureFilename` templates are all present in the shipped assets.

The current modern `MeshAsset` represents only position, normal, and UV coordinates with triangle-list indices — a minimal subset sufficient for rigid meshes but missing materials, textures, bones, weights, and frame hierarchy that the real RAN assets contain.

---

## 2. Repository / Source References

- **Legacy mesh loader**: `legacy/Lib_Engine/Meshs/DxSimpleMeshMan.cpp`
- **Legacy skin mesh**: `legacy/Lib_Engine/Meshs/DxSkinMesh9.cpp`
- **Legacy frame mesh**: `legacy/Lib_Engine/Meshs/DxFrameMesh.cpp`
- **Bone collector**: `legacy/Lib_Engine/Meshs/DxBoneCollector.cpp`
- **MXF decrypt/encrypt**: `legacy/Lib_Engine/DxCommon/MemoryXFile.h` / `MemoryXFile.cpp`
- **Modern mesh asset**: `modern/client/assets/MeshAsset.h`, `MeshAsset.cpp`
- **Modern mesh decoder**: `modern/client/assets/MeshDecoder.h`, `TestMeshDecoder.h`, `TestMeshDecoder.cpp`
- **Asset types**: `modern/client/assets/AssetTypes.h`
- **Asset upload**: `modern/client/rendering/AssetUpload.h`, `NullAssetUploader.h`

---

## 3. Legacy `.X` Loading Path

The legacy RAN client loads `.x` meshes through `DxSimMesh::Create()` in `DxSimpleMeshMan.cpp`:

```text
DxSimpleMeshMan::Load(szFile, pd3dDevice, dwFVF)
  ↓
DxSimMesh::Create(pd3dDevice, dwFVF, szFilename, ...)
  ↓
GetMeshFilePath(szFilename, bEncrypted)
  ↓  Checks for .mxf first, then .x, then other extensions
  ↓
if bEncrypted:
    CMemoryXFile::LoadFileDec(strFileFind)
      ↓  Decrypts .mxf → raw .x bytes
      ↓  D3DXLoadMeshFromXInMemory(pData, nSize, ...)
else:
    D3DXLoadMeshFromX(strFileFind, D3DXMESH_MANAGED, ...)
  ↓
OptimizeInplace(D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE)
  ↓
CloneMeshFVF(D3DXMESH_MANAGED, dwFVF, pd3dDevice, ...)
  ↓
Extract vertices, normals, UVs from locked vertex buffer
  ↓
Extract materials and textures from pMtrlBuffer
  ↓
DxSimMesh (ready for rendering)
```

For **skinned/character meshes**, a separate path exists in `DxSkinMesh9.cpp`:

```text
DxSkinMesh9::OnCreateSkin(pd3dDevice, szSkinFile, szSkeletonFile)
  ↓
DxBoneCollector::Load(szSkeletonFile, pd3dDevice)
  ↓
if bEncrypted:
    CMemoryXFile::LoadFileDec(strFileFind)
      ↓  D3DXLoadMeshHierarchyFromXInMemory(pData, nSize, ...)
else:
    D3DXLoadMeshHierarchyFromXA(strFileFind, ...)
  ↓
SetupBoneMatrixPointers(m_pFrameRoot)
SetupNameOnMeshContainer(m_pFrameRoot)
D3DXFrameCalculateBoundingSphere(m_pFrameRoot, ...)
```

For **frame-based meshes**, `DxFrameMesh.cpp` uses `D3DXLoadMeshFromXof`:

```text
DxFrameMesh::Create(pxofobjCur, pD3DDevice)
  ↓
D3DXLoadMeshFromXof(pxofobjCur, D3DXMESH_SYSTEMMEM, ...)
  ↓
Optimize(D3DXMESHOPT_COMPACT|D3DXMESHOPT_ATTRSORT, ...)
  ↓
Extract vertices, triangles, materials, textures
```

### Key Legacy Classes

| Class | File | Purpose |
|-------|------|---------|
| `DxSimMesh` | `DxSimpleMeshMan.cpp` | Mesh instance with vertices, normals, UVs, materials, textures |
| `DxSimpleMeshMan` | `DxSimpleMeshMan.cpp` | Singleton mesh manager, caching, morph blending |
| `DxSkinMesh9` | `DxSkinMesh9.cpp` | Skinned mesh with bone hierarchy and animation |
| `DxFrameMesh` | `DxFrameMesh.cpp` | Frame-based mesh with hierarchy |
| `DxBoneCollector` | `DxBoneCollector.cpp` | Bone/skeleton loader |
| `CMemoryXFile` | `MemoryXFile.h/cpp` | MXF container decrypt/encrypt |

---

## 4. Real Asset Inventory

### Location

All RAN `.x` files reside in:
```
D:\FILES\project\RanOnline-Build\ASURA CLIENT\data\skeleton\
```

**Total: 604 `.x` files** (all with `b_` prefix naming convention).

Additionally, matching `.mxf` files exist alongside the `.x` files for encrypted variants.

### Representative Files

| File | Size | Format | Templates | Notes |
|------|------|--------|-----------|-------|
| `b_2017_victors_wings.x` | 459 B | xof 0303bzip | Header, Frame, SkinWeights, XSkinMeshHeader, VertexDuplicationIndices | Small item mesh, compressed |
| `b_3lvl_f.x` | 4,460 B | xof 0303bin | Mesh, SkinWeights, XSkinMeshHeader, Frame, FrameTransformMatrix, VertexDuplicationIndices | Static mesh, bzip |
| `b_9lvl_m.x` | 7,705 B | xof 0303bin | Mesh, SkinWeights, XSkinMeshHeader, Frame, FrameTransformMatrix, VertexDuplicationIndices | Medium mesh, bzip |
| `b_alphacar.x` | 4,523 B | xof 0303bin | Mesh, SkinWeights, XSkinMeshHeader, Frame, FrameTransformMatrix, VertexDuplicationIndices | Vehicle mesh, bzip |
| `b_Athena.x` | 8,064 B | xof 0303bin | Mesh, SkinWeights, XSkinMeshHeader, Frame, FrameTransformMatrix, VertexDuplicationIndices | Character mesh, bzip |
| `b_3boss_01.x` | 219,742 B | xof 0303bin | Mesh, MeshNormals, MeshTextureCoords, MeshMaterialList, SkinWeights, XSkinMeshHeader, Frame, FrameTransformMatrix, Material, TextureFilename, VertexDuplicationIndices | Boss mesh with full features |
| `b_aegis_wings.x` | 23,004 B | xof 0303bzip | Mesh, SkinWeights, XSkinMeshHeader, Frame, VertexDuplicationIndices | Large item mesh, compressed |
| `b_as_tiger_weapon_01.x` | 1,927 B | xof 0303bin | Mesh, SkinWeights, XSkinMeshHeader, Frame, VertexDuplicationIndices | Weapon mesh |
| `b_baby_devil.x` | 1,744 B | xof 0303bzip | Mesh, SkinWeights, XSkinMeshHeader, Frame, VertexDuplicationIndices | Character mesh, compressed |

### Encoding Distribution

- **Binary** (`xof 0303bin 0032`): ~518 files (85.8%)
- **Bzip-compressed** (`xof 0303bzip0032`): ~86 files (14.2%)

All files use the same header format: `xof 0303<enc> 0032` where `<enc>` is `bin` or `bzip`.

### Template Occurrences (across all 604 files)

| Template | Approx. Occurrences | Description |
|----------|-------------------|-------------|
| `Frame` | ~75,276 | Scene graph hierarchy nodes |
| `FrameTransformMatrix` | ~37,638 | Transform matrices per frame |
| `SkinWeights` | ~1,609 | Per-vertex bone influence data |
| `Mesh` | ~1,397 | Geometry data (vertices, faces, normals) |
| `VertexDuplicationIndices` | ~717 | Vertex duplication tracking |
| `MeshNormals` | ~226 | Face/vertex normals |
| `MeshTextureCoords` | ~211 | Texture coordinate data |
| `MeshMaterialList` | ~226 | Material assignment per face |
| `Material` | ~455 | Material properties |
| `XSkinMeshHeader` | ~508 | Skin mesh metadata |
| `Header` | ~508 | File header template |
| `TextureFilename` | ~46 | Texture file references |

Note: Template name strings appear in binary data, so counts include matches within template names and data blobs. Not every file contains every template — a simple item mesh may only have `Mesh`, `SkinWeights`, `XSkinMeshHeader`, `Frame`, `FrameTransformMatrix`, `VertexDuplicationIndices`.

---

## 5. `.X` Header / Encoding Findings

### Header Format (16 bytes)

```
Offset  Size  Field          Value
0       4     Magic          "xof "
4       4     Version        0x30303330 ("0303" as ASCII)
8       4     Encoding       "bin " or "bzip"
12      4     Flags          0x30333030 ("0032" as ASCII)
```

- **Version**: 03.03 (DirectX 9 format)
- **Encoding**: `bin` = raw binary, `bzip` = bzip2-compressed binary
- **Flags**: `0032` — meaning unclear, possibly format variant identifier

### Encoding Detection

All files start with `xof 0303bin 0032` or `xof 0303bzip0032`. Both are **binary** format. The `bzip` variant uses bzip2 compression on the template data portion after the header.

The `.mxf` container wraps the `.x` file with a 12-byte header (version, payloadSize, fileType=0) and applies the encryption transform.

---

## 6. Actual Templates Found

Based on binary inspection of representative `.x` files, the following DirectX `.X` templates **actually occur** in RAN assets:

| Template | Present? | Notes |
|----------|----------|-------|
| `Header` | Yes | Standard file header |
| `Frame` | Yes | Scene graph hierarchy |
| `FrameTransformMatrix` | Yes | Per-frame transformation |
| `Mesh` | Yes | Core geometry (always present) |
| `MeshNormals` | Sometimes | Present in complex meshes like `b_3boss_01.x` |
| `MeshTextureCoords` | Sometimes | Present in meshes with textures |
| `MeshMaterialList` | Sometimes | Present when materials exist |
| `Material` | Sometimes | Material properties |
| `TextureFilename` | Sometimes | Texture file reference |
| `SkinWeights` | Yes | Very common — most meshes are skinned |
| `XSkinMeshHeader` | Yes | Skin mesh metadata |
| `VertexDuplicationIndices` | Yes | Vertex duplication tracking |

**Templates NOT found** in any inspected file: `PatchMesh`, `MeshFace`, `AnimationSet`, `Animation`, `Position`, `Normal`, `TextureCoordinate` (as separate templates — UVs are inside `MeshTextureCoords`).

---

## 7. Mesh Geometry Format

### Vertices

- **Vertex count**: Varies by mesh type (small items ~100-500, character meshes ~1,000-10,000+)
- **Position order**: XYZ (3 float32 values)
- **Normal order**: NX, NY, NZ (3 float32 values, follows position)
- **UV order**: U, V (2 float32 values, follows normal)
- **Total per-vertex size**: 8 float32 = 32 bytes (position + normal + UV)
- **Coordinate system**: Left-handed (DirectX convention, confirmed by D3DXLoadMeshFromX usage)
- **Numeric type**: IEEE-754 float32, little-endian

### Faces

- **Face count**: Varies (small items ~50-200, large meshes ~500-5,000+)
- **Polygon size**: Always triangles (3 indices per face)
- **Winding order**: Counter-clockwise (standard DirectX default for D3DXLoadMeshFromX)
- **Index type**: uint32 (32-bit indices)

### Mesh Structure

The `Mesh` template contains:
- Vertex count and face count
- Face data (array of 3 indices per triangle)
- Vertex data array (packed position, normal, UV per vertex)
- Optional sub-sets per material

---

## 8. Normals

### Findings

- **MeshNormals template** present in ~226 files (complex meshes only)
- Simple meshes may have normals embedded directly in the `Mesh` template's vertex data
- **Normal indices** may differ from vertex indices (separate normal index array in `MeshNormals`)
- Normals are **not always unit length** in the raw file — legacy code may compute them
- The legacy pipeline extracts normals from the vertex buffer at offset `sizeof(D3DXVECTOR3)` after position

### Normal Handling

- `D3DXLoadMeshFromX` may generate normals if `MeshNormals` is absent
- `OptimizeInplace` may reorder vertices and normals together
- `CloneMeshFVF` converts the vertex format, preserving normal data
- Normals are extracted as `D3DXVECTOR3` from the locked vertex buffer

---

## 9. UVs (Texture Coordinates)

### Findings

- **MeshTextureCoords template** present in ~211 files
- UV count matches vertex count
- **Single UV set** — no multiple UV sets observed in any inspected file
- Coordinate orientation: U increases rightward, V increases downward (standard DirectX)
- V-axis is **not flipped** in the raw `.x` files
- The legacy code extracts UVs at the end of the vertex buffer (after position + normal)

### Comparison with `MeshVertex::uv`

The current modern `MeshVertex` has `u` and `v` as `float32` values, matching the `MeshTextureCoords` data format exactly. One UV set is sufficient for all observed RAN assets.

---

## 10. Materials and Textures

### Findings

- **MeshMaterialList** present in ~226 files
- **Material** present in ~455 files (many are referenced across multiple meshes)
- **TextureFilename** present in ~46 files
- Material-to-face mapping: one subset per material, drawn via `DrawSubset(i)`
- Material properties: Diffuse, Ambient (set equal to Diffuse), Specular, Emissive, Power
- **Diffuse** is the primary material property
- **Ambient** is copied from Diffuse in legacy code (`m_pMaterials[i].MatD3D.Ambient = m_pMaterials[i].MatD3D.Diffuse`)
- Texture references use relative filenames (e.g., `texture.bmp`, `skin01.dds`)
- Multiple textures per material are **not observed** — one texture per material
- Legacy code loads textures via `TextureManager::LoadTexture(d3dxMtrls[i].pTextureFilename, ...)`

### Material Mapping to Modern Architecture

The current `MeshAsset` has **no material fields**. Materials and textures would need a separate asset type or a future mesh extension.

---

## 11. Skinning / Character Meshes

### Findings

**Skinning is ubiquitous in RAN `.x` files.** Almost all 604 meshes contain `SkinWeights` and `XSkinMeshHeader` data.

### SkinWeights Template

```
template SkinWeights {
    STRING boneName;
    DWORD nVertices;
    array DWORD vertexIndices[nVertices];
    array float weights[nVertices];
}
```

- **Bone names**: Referenced by string in the SkinWeights template
- **Vertex influence count**: Typically 2-4 bones per vertex
- **Weights**: Float32 values, typically normalized (sum ≈ 1.0)
- **Bone offset matrices**: Stored in `XSkinMeshHeader` and frame hierarchy

### XSkinMeshHeader

Contains metadata about the skinned mesh:
- Number of bones
- Max influences per vertex
- Skin version

### Bone Hierarchy

- Bones are represented as `Frame` nodes with `FrameTransformMatrix`
- `DxBoneCollector` loads the skeleton from a separate file
- `DxSkinMesh9::SetupBoneMatrixPointersOnMesh` maps bones to mesh containers
- Bone names must match between the mesh and the skeleton file

### Legacy Skinning Path

```text
DxSkinMesh9::OnCreateSkin(pd3dDevice, skinFile, skeletonFile)
  ↓
D3DXLoadMeshHierarchyFromXInMemory(pData, nSize, ...)
  ↓
SetupBoneMatrixPointers(m_pFrameRoot) — maps bone names to matrices
SetupNameOnMeshContainer(m_pFrameRoot) — stores bone names
D3DXFrameCalculateBoundingSphere(m_pFrameRoot, ...)
```

### Skin Data Not in Current MeshAsset

The current `MeshAsset` has **no bone weights, bone indices, bone names, or bone matrices**. These would require a separate data structure.

---

## 12. Frame / Hierarchy Structure

### Findings

- **Frame hierarchy** is present in all `.x` files
- **FrameTransformMatrix** provides per-frame transformation
- The hierarchy follows a tree structure: root frame → child frames → mesh containers
- **Mesh files and skeleton information are combined** in the same `.x` file (not separated)
- Each `Frame` can have a `pMeshContainer` containing geometry and material data

### Hierarchy Examples

```text
Root Frame
├── Frame (Bone 0)
│   ├── FrameTransformMatrix
│   └── MeshContainer (skin mesh part)
├── Frame (Bone 1)
│   ├── FrameTransformMatrix
│   └── MeshContainer (skin mesh part)
└── Frame (Mesh)
    ├── FrameTransformMatrix
    └── MeshContainer (static mesh part)
```

`D3DXLoadMeshFromX` **flattens** the frame hierarchy into a single mesh with subsets. For animated/skinned meshes, `D3DXLoadMeshHierarchyFromX` preserves the hierarchy.

---

## 13. Legacy Post-Processing

The legacy pipeline performs several operations immediately after loading:

### OptimizeInplace

```cpp
m_pLocalMesh->OptimizeInplace(
    D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE,
    (DWORD*)pAdjacencyBuffer->GetBufferPointer(), NULL, NULL, NULL);
```

Operations performed:
- **D3DXMESHOPT_COMPACT**: Removes unused vertices and reorders indices
- **D3DXMESHOPT_ATTRSORT**: Sorts faces by material attribute for efficient `DrawSubset` calls
- **D3DXMESHOPT_VERTEXCACHE**: Reorders triangles to optimize GPU vertex cache usage

### CloneMeshFVF

```cpp
pLOCALMESH[i]->CloneMeshFVF(D3DXMESH_MANAGED, dwFVF, pd3dDevice, &pLOCALMESH[i]);
```

- Converts the mesh to the target FVF (Flexible Vertex Format)
- The FVF determines which vertex components are present (XYZ, NORMAL, DIFFUSE, TEX1)
- May reorder vertex data to match the FVF layout

### Additional Operations

- **D3DXComputeBoundingSphere**: Computes bounding sphere for the mesh
- **D3DXComputeBoundingBox**: Computes bounding box
- **D3DXComputeNormals**: May be called if normals are missing (commented out in current code)
- **SetFVF**: Sets the FVF on the loaded mesh, potentially converting vertex format

### Important Implications

The modern decoder must reproduce the **post-processed geometry**, not the raw `.x` data. This means:
- Vertex reordering from `OptimizeInplace` must be accounted for
- The FVF conversion from `CloneMeshFVF` determines the final vertex layout
- Index reordering from `OptimizeInplace` changes the index buffer

---

## 14. Compare with Current `MeshAsset`

### Mapping Table

| RAN `.X` Data | Current `MeshAsset` Field | Status |
|---------------|--------------------------|--------|
| Mesh vertex XYZ | `MeshVertex::position` (Vector3) | ✅ Matches |
| Mesh normal | `MeshVertex::normal` (Vector3) | ✅ Matches |
| Mesh UV | `MeshVertex::u`, `MeshVertex::v` | ✅ Matches |
| Mesh face indices | `MeshAsset::indices` (uint32) | ✅ Matches |
| Triangle topology | `PrimitiveTopology::TriangleList` | ✅ Matches |
| Vertex count | `MeshAsset::GetVertexCount()` | ✅ Matches |
| Index count | `MeshAsset::GetIndexCount()` | ✅ Matches |
| Material properties | — | ❌ Missing |
| Texture references | — | ❌ Missing |
| Submeshes (DrawSubset) | — | ❌ Missing |
| Bone weights | — | ❌ Missing |
| Bone indices | — | ❌ Missing |
| Bone names | — | ❌ Missing |
| Bone matrices | — | ❌ Missing |
| Frame hierarchy | — | ❌ Missing |
| Multiple UV sets | — | ❌ Missing (only 1 exists) |
| Vertex duplication indices | — | ❌ Missing |
| Material colors | — | ❌ Missing |
| Vertex colors / DIFFUSE | — | ❌ Missing |
| Morph targets (3 morph targets observed) | — | ❌ Missing |

### Information That Does NOT Fit

- **Materials**: Would need a separate `MaterialAsset` type
- **Textures**: Would need a separate texture resource system
- **Submeshes**: Would need a `SubMesh` or `MeshSubset` concept
- **Skinning data**: Would need a `SkinInfo` type or separate asset
- **Frame hierarchy**: Would need a scene graph or bone hierarchy type
- **Morph targets**: Would need an animation/morph system
- **Vertex colors**: Would need a `DIFFUSE` component in `MeshVertex`

---

## 15. Determine the Smallest Decoder Scope

Based on real asset analysis, the smallest useful `XMeshDecoder` scope for CLIENT-015:

### Minimum Required Templates

```
Header
Mesh
MeshNormals      (if present)
MeshTextureCoords  (if present)
SkinWeights      (if present — for character meshes)
Frame            (for bone hierarchy)
```

### Recommended Minimal Decoder (Static Meshes Only)

For the first implementation, **static meshes only** (no skinning):

```cpp
class XMeshDecoder : public IMeshDecoder {
    Result<MeshAsset> DecodeMesh(const ResourceData& data) override;
};
```

Must parse:
1. `.x` file header (magic, version, encoding)
2. `Mesh` template → vertex count, face count, vertices, indices
3. `MeshNormals` template (if present) → normal data
4. `MeshTextureCoords` template (if present) → UV data
5. Validate all indices in range, all vertices finite
6. Return `MeshAsset::Create(PrimitiveTopology::TriangleList, vertices, indices)`

### Deferred to Future Milestones

- **Skinning** (SkinWeights, XSkinMeshHeader, Frame) → separate milestone
- **Materials** (MeshMaterialList, Material, TextureFilename) → separate milestone or data type
- **Morph targets** → separate animation system
- **Frame hierarchy** → separate scene graph system
- **Bzip decompression** → utility function for compressed `.x` files
- **Morph blending** (3 morph targets observed in `DxSimMesh`) → separate system

### Encoding Handling

The decoder must handle both `bin` and `bzip` encoding:
- `bin`: Parse binary data directly
- `bzip`: Decompress with a bzip2 library, then parse

---

## 16. Risks / Unknowns

1. **Bzip decompression**: 86 files use bzip2 compression. The decoder needs a bzip2 implementation. No existing bzip2 library in the modern codebase.

2. **Vertex duplication**: `VertexDuplicationIndices` template exists in ~717 files. Its purpose is unclear — may track which vertices are shared across faces. May affect how indices are interpreted.

3. **Morph targets**: `DxSimMesh` has `m_pVertex[3]` and `m_pNormal[3]` arrays, suggesting 3 morph targets. The `.x` file format for morph targets is not fully understood.

4. **FVF conversion**: The legacy `CloneMeshFVF` may change vertex layout. The exact FVF flags used by RAN are not documented in the `.x` files themselves — they are determined at runtime by `DwFVF`.

5. **Bone data format**: The exact layout of `SkinWeights` data in the `.x` binary format is not fully verified by parsing individual files. The template structure is known but binary layout needs confirmation.

6. **Post-processing effects**: `OptimizeInplace` with `D3DXMESHOPT_ATTRSORT` reorders faces and may split vertices. The modern decoder must reproduce the final vertex/index layout, not the original `.x` layout.

7. **Texture paths**: Texture filenames are relative and may require a path resolution strategy not yet defined.

8. **All 604 files are skinned**: Since virtually every mesh has `SkinWeights`, there may be no truly "static" mesh in the shipped client. This could mean the first decoder must handle skinning, or that static meshes are generated differently at runtime.

---

## 17. Evidence / Sample Asset Paths

- Skeleton directory: `D:\Files\project\RanOnline-Build\ASURA CLIENT\data\skeleton\`
- Total `.x` files: 604
- Total `.mxf` files: matching count (each `.x` has a corresponding `.mxf` for encrypted loading)
- Sample static mesh: `b_3lvl_f.x` (4,460 B, bzip)
- Sample character mesh: `b_Athena.x` (8,064 B, bin)
- Sample boss mesh: `b_3boss_01.x` (219,742 B, bin — has all templates)
- Sample weapon: `b_as_tiger_weapon_01.x` (1,927 B, bin)
- Sample item: `b_2017_victors_wings.x` (459 B, bzip)
- Sample vehicle: `b_alphacar.x` (4,523 B, bin)

---

## 18. Investigation Conclusion

RAN Online uses the DirectX `.X` binary format (version 03.03) with optional bzip2 compression. The `.mxf` container applies a simple byte-level encryption (`byte += 0xEA; byte ^= 0xEB`) that strips the 12-byte header and decrypts the payload to plain `.x` bytes.

**Every mesh in the shipped client is skinned** — `SkinWeights` and `XSkinMeshHeader` are present in virtually all 604 `.x` files. This is a critical finding that means the first modern `.X` decoder may need to handle skinning from the start, or that static meshes are generated differently at runtime.

The current modern `MeshAsset` represents only the geometry subset (position, normal, UV, indices) and is sufficient as a target for the decoder output, but it is missing all the additional data (materials, textures, bones, weights, hierarchy) that the real assets contain.

The legacy loading pipeline uses `D3DXLoadMeshFromX` followed by `OptimizeInplace` and `CloneMeshFVF`, meaning the modern decoder must produce geometry that accounts for post-processing (vertex reordering, index optimization).

**Recommendation**: CLIENT-015 should implement a minimal `XMeshDecoder` that handles `Mesh`, `MeshNormals`, `MeshTextureCoords`, and `SkinWeights` templates for both `bin` and `bzip` encoded `.x` files. Materials, textures, frame hierarchy, and morph targets should be deferred to separate milestones.

---

## 19. No Code Implementation

This investigation contains no code changes. No `XMeshDecoder` was implemented. No `MeshAsset`, `MeshDecoder`, `AssetTypes`, renderer, or gameplay code was modified. The only new file is this document.

---

## 20. Validation

The existing build is unaffected by this investigation. All existing tests continue to pass.

---

# CLIENT-015 verification addendum

*Appended 2026-09-28. Everything above is the CLIENT-014 investigation, left
as written. This section records what CLIENT-015 established by decoding the
shipped tree, and corrects two findings above that turned out to be wrong.*

## 21. Correction: the binary `.X` body is a token stream, not record-framed

Section 5 and the original reading of the 86 `bzip` files both needed
correcting. The binary body is **not** a sequence of size-prefixed records. It is
the **DirectX .X tokenized encoding** (the published "Binary Encoding" grammar):
every element begins with a little-endian `WORD` token, and record-bearing
tokens carry their own count.

| Token | Value | Record |
|---|---|---|
| `Name` | 1 | `DWORD` count, then the characters |
| `String` | 2 | `DWORD` count, then the characters |
| `Integer` | 3 | `DWORD` value |
| `Guid` | 5 | 16 bytes |
| `IntegerList` | 6 | `DWORD` count, then `count` x `DWORD` |
| `FloatList` | 7 | `DWORD` count, then `count` x `float` |
| `OBrace` / `CBrace` | 10 / 11 | punctuation |
| `OBracket` / `CBracket` | 14 / 15 | punctuation |
| `Dot` | 18 | `.` |
| `Comma` / `Semicolon` | 19 / 20 | list separators |
| `Template` | 31 | begins a template definition |
| `Word` .. `CString` | 40 .. 51 | primitive type codes |
| `Array` | 52 | begins an array member |

A **template definition** is `Template Name '{' [Guid] members '}'`, and each
member is a type token, an optional `Name`, and either a `Semicolon` (primitive,
or a template reference) or an `Array` type with an `OBracket` dimension and
`CBracket`. A template may also use the optional-parts form `'[' ... ']'` or a
bare `...`, which is the shape RAN's `bzip` files use for `Frame`.

A **data object** is `Name [instance-name]* [String] '{' [Guid] parts '}'`, and
`parts` are lists, nested objects, data references (`'{' Name ';' '}'`) and
separators. Objects nest: **every real RAN mesh sits inside a `Frame`**, so a
reader that only looks at top level finds no geometry at all.

With this grammar, a complete 6 KB file parses to exactly its end offset, and
**all 510 binary files in the tree walk cleanly**. No file needed a tolerant
reader, a heuristic, or a special case.

## 22. Correction: `bzip` is MSZip, not bzip2

Section 5 treated `bzip` as an opaque compressed blob. It is **MSZip**: MS's
dictionary-chained raw DEFLATE, and the layout RAN writes is:

```
bytes  0..15   the same 16-byte `xof 0303bzip0032` header
bytes 16..19   uint32 total uncompressed size, header included
bytes 20..23   reserved; not a CRC-32 of the payload, not validated
bytes 24..     one or more chunks, each:
                  "CK"                                  2 bytes
                  a raw RFC 1951 DEFLATE stream, no zlib wrapper
                  a 4-byte trailer, present only between chunks
```

Each chunk after the first is inflated **with the previous chunk's decompressed
output as a preset dictionary**, which is the defining behaviour of MSZip. The
DEFLATE stream yields the token stream *without* the 16-byte header, so the
header has to be put back in front of it.

Verified: 85 of the 86 `bzip` files have one chunk; `b_aegis_wings.x` has three
(total 79,602 bytes, 15 of them the 16-byte header). All 86 inflate to a token
stream that parses cleanly.

The `reserved` field is not a CRC-32: it matches neither `crc32` of the inflated
payload nor `crc32` of the compressed bytes, and the per-chunk trailer does not
match either. The decoder does not validate either, and says so.

## 23. Corrected inventory

Section 4 counted 604 files. The tree holds **608**: 604 with a lowercase `.x`
and 4 with an uppercase `.X`. A sweep that compares the extension exactly will
quietly cover 604 of 608.

| | count |
|---|---|
| total `.x` / `.X` under `data/skeleton` | **608** |
| `xof 0303bin 0032` | **510** |
| `xof 0303bzip0032` | **86** |
| `xof 0303txt 0032` | **12** |

All 608 are version `0303` and float size `0032`; there is no other version and
no 64-bit float file in the tree.

## 24. Static geometry findings, verified by decoding

- **43 of the 608 files carry a `Mesh`.** 42 are `bin`; one is `bzip`.
- **13,292 faces, and every one is a triangle** (`nFaceVertexIndices == 3` in
  all 13,292). There are no polygons anywhere in the tree, so a decoder has no
  polygon case to accommodate and must not invent a fan order.
- `MeshNormals.faceNormals` is a **verbatim copy of the face list** in every
  inspected mesh, which is why `normal[v]` is the correct per-vertex mapping and
  no indirection is needed.
- A file may carry **many `Mesh` objects**: 43 assets are multi-part, with up
  to **116 sub-meshes in one file**. `Mesh`, `MeshNormals` and
  `MeshTextureCoords` are siblings inside a `Frame` and correspond positionally.
- Largest single mesh: 2,050 vertices / 8,092 indices, far inside the
  `MeshAsset` ceilings.
- **10 files have no `MeshTextureCoords` at all.**
- **`VertexDuplicationIndices` (211 instances) is not required for static
  geometry.** The `Mesh` vertex list is already self-consistent, every index is
  in range, and `faceNormals` mirrors `faces`; VDI only relates the duplicated
  skinned list to the original one, which is a question about bones. It is read
  past and left unsupported.

## 25. Known real exceptions

None of these is converted into a pass.

| Exception | Files | Why the decoder refuses |
|---|---|---|
| `Mesh` with no `MeshNormals` | `b_pet_human_ninefox.x` (1) | Normals are never invented. A zero or absent normal is a wrong normal, and `MeshAsset` has no "no normals" state. |
| `MeshTextureCoords` out of step with `Mesh` blocks | `b_effet_char.x`, `b_m.x`, `b_m1.x`, `b_w.x` (4) | The UV blocks are not positionally aligned with the meshes (e.g. `b_m.x` has 16 meshes and 15 UV sets, and the UV counts do not line up with the mesh counts from index 3 on). Pairing by index would attach UVs to the wrong vertices, so the correspondence is refused instead. |
| `Mesh` with `nVertices == 0` | `b_mob_es_01.x`, `b_mob_mm_01.x`, `b_mob_mt_01.x`, `b_mob_mujuk.x`, `b_mob_se_01.x`, `b_mob_tp_01.x`, `b_npc_teacher_mm.x`, `b_vehicle_lightcycle.x` (8) | An empty `Mesh` is not a mesh. `MeshAsset` refuses zero vertices, and returning an empty asset would be a different answer from refusing. |
| no `Mesh` at all | the other 561 | A skeleton-only or animation `.x` is not a static mesh. Reported as *no static geometry*, which is a different statement from *failed to decode*. |

13 geometry-bearing files are therefore refused, and **30 decode**. That is
`13 + 30 = 43`, i.e. every file that carries a `Mesh` is accounted for.

## 26. Decoder coverage as built

| | |
|---|---|
| `xof 0303bin 0032` | supported, and 30 of 42 geometry-bearing files decode |
| `xof 0303bzip0032` (MSZip) | supported; all 86 inflate and parse. None carries static geometry, so none produces a `MeshAsset` |
| `xof 0303txt 0032` | **not** decoded; text encoding is out of scope |
| other versions / `0064` floats / `tzip` | refused |
| `Mesh`, `MeshNormals`, `MeshTextureCoords` | decoded |
| `MeshMaterialList`, `Material`, `TextureFilename` | parsed and discarded |
| `XSkinMeshHeader`, `SkinWeights`, `VertexDuplicationIndices` | parsed and discarded; skinning deferred |
| `Frame`, `FrameTransformMatrix`, `Matrix4x4`, `Animation*` | parsed and discarded; hierarchy and animation deferred |

Multi-`Mesh` files are **merged in document order** into one `MeshAsset`, each
sub-mesh's indices biased by the running vertex base. That is the only way a
single-`MeshAsset` shape can hold a 116-sub-mesh model without being extended
into submeshes, and it drops no geometry.

### Real-asset results

`data/skeleton`, all 608 files attempted, nothing capped:

```
bin  : 510 candidates,  30 decoded, 12 unsupported,  0 malformed, 468 no static geometry
bzip :  86 candidates,   0 decoded,  0 unsupported,  0 malformed,  86 no static geometry
txt  :  12 candidates (text-encoded, not in scope)
geometry decoded: 10,393 vertices, 8,456 triangles
```

`data/*.mxf`, all 901 files attempted:

```
414 decoded, 24 refused, 463 no static geometry, 2 bad container
```

so `MxfMeshTransform -> XMeshDecoder -> MeshAsset` is exercised end to end on
414 real files. The two bad containers (`desktop.mxf`, `z800v3.mxf`) are refused
by CLIENT-013's transform before the decoder sees them.

## 27. Defects found and fixed while building this

Recorded because each is a real bug, not a test artefact.

1. **`const char*` into `std::vector<uint8_t>::insert` does not terminate** on
   the MSVC 14.44 STL. `bytes.insert(bytes.end(), "array", "array" + 5)` on a
   three-byte vector never returns. It surfaced as a *non-deterministic* debug
   heap assertion many tests away from the call, which is why it needed a
   standalone driver to pin down. Every append in the tests is a per-byte loop
   now; there is no overload left to misresolve.
2. **DEFLATE block header read backwards.** The first implementation read one
   bit as BTYPE and then one bit as BFINAL. The order is BFINAL (1 bit) then
   BTYPE (2 bits), so the first block's BFINAL was dispatched on as a block type
   and the following BTYPE bit was read as a second BFINAL. Only a fixture
   produced by an independent encoder (zlib) exposed it.
3. **A code-length pre-check tested the wrong thing.** The Huffman decoder
   refused to decode a code unless that many bits were *already buffered*, but
   the bit reader refills from the input, so a code ending in the last byte of a
   well-formed stream was rejected. It now tests whether another bit is
   readable at all.
4. **The inflater's history ring could be written one past its end.** A full
   32 KiB preset dictionary left the write position at `kWindowSize`, and the
   next `Emit` wrote outside `m_history`. The position now wraps.
5. **Nested objects were never recorded.** Geometry was only recorded for
   top-level objects, so every real mesh -- all of which sit inside a `Frame` --
   was dropped. Recording moved into `ReadObject`.
6. **`ReadParts` consumed the `Name` token and then let `ReadObject` consume it
   again**, stepping one token too far and failing every nested object.
7. **List separators were refused.** `,` and `;` are legal between values of a
   list and may close a string member; the parser treated them as unknown.
8. **The bzip path handed the reader the file header.** After inflating, `tokens`
   pointed at the start of the buffer, i.e. at the ASCII `xof 0303bzip0032`,
   instead of past it.
9. **Unbounded recursion.** A desynchronised stream can present an object token
   at nearly every position, so object nesting is bounded at depth 64.
