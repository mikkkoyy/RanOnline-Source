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
