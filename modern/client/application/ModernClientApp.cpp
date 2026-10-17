// ASURA-CHARACTER-001B: the modern client production executable.
//
// ModernClientApp wires the real client stack into one windowed program:
//
//   Win32PlatformEvents  -> the native 960x540 window and its message pump
//   Win32InputSource     -> Win32 input translated to platform-independent events
//   D3D11Renderer        -> the Direct3D 11 backend (real frames, real Present)
//   D3D11AssetUploader   -> CPU assets uploaded to real GPU buffers/textures
//   asset pipeline       -> [mxf | plain .X] -> RanMeshSource -> XMeshDecoder
//                           -> MeshAsset -> GPU upload
//   Application          -> the frame loop, with a static character draw
//
// Two meshes can be rendered, and which one is a runtime choice:
//
//   * The default is the CLIENT-RENDER-002 regression asset,
//     data/skin/m_luffy_timeskip.mxf: an `.mxf` container, whose materials
//     name real PNG files (blackcatyb_luffy_timeskip.png on the body,
//     blackcatyb_luffy_timeskip_eye.png on the eye).
//
//   * ASURA-CHARACTER-001B adds an ORIGINAL RAN playable character: the
//     SacredGate Male Brawler base body, data/skin/s_m_hb.X. It is a plain
//     DirectX `.X` with no container at all, so RanMeshSource routes it
//     past the `.mxf` transform unchanged. Its materials name real DDS
//     files (m1_hb_body.dds, m1_khb_foot.dds, ...), resolved by name over
//     the ASURA texture tree.
//
//   The mesh is selected by --skin-piece (a RAN wearing file under
//   data/skinobject/, e.g. CUN0001_m.cps, whose mesh field names the body)
//   or by --mesh (an explicit path relative to the asset root).
//
// What is drawn every frame: one indexed draw per MeshSection, each binding
// its own material's texture at its own index range. Every *unique* texture
// is decoded and uploaded exactly once and the handle is shared. A material
// that names no texture, or whose named texture cannot be resolved, decoded
// or uploaded, falls back to a clearly-labelled diagnostic color and is
// reported as such -- it is never counted as a textured draw, and a run
// whose named textures did not all load exits non-zero.
//
// Usage:
//   ModernClientApp.exe [--asset-root <path>] [--mesh <path>]
//                       [--skin-piece <path>]
//
// --asset-root names the RAN client tree. Without it, RAN_ASSET_ROOT is
// tried, then the current directory. Escape or closing the window exits;
// the process returns 0 on a clean stop and non-zero on any failure.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
// Must be undef'd BEFORE any project header: Result.h declares a member
// named GetMessage(), and the Windows macro would rename that declaration
// too, leaving Status with GetMessageA instead of GetMessage.
#undef GetMessage

#include "application/Application.h"
#include "assets/ImageAsset.h"
#include "assets/MeshAsset.h"
#include "assets/MxfMeshTransform.h"
#include "assets/RanMaterialSlot.h"
#include "assets/RanMeshSource.h"
#include "assets/SkinPieceContainer.h"
#include "assets/TextureCache.h"
#include "assets/WorldTerrainMesh.h"
#include "assets/TextureLoader.h"
#include "assets/XMeshDecoder.h"
#include "gameplay/ClientCharacterController.h"
#include "input/InputEvents.h"
#include "input/InputSystem.h"
#include "input/platform/Win32InputSource.h"
#include "input/platform/Win32PlatformEvents.h"
#include "math/Vector3.h"
#include "WorldStaticMesh.h"
#include "rendering/CameraMatrices.h"
#include "rendering/D3D11AssetUploader.h"
#include "rendering/D3D11Renderer.h"
#include "rendering/Renderer.h"
#include "rendering/RenderingTypes.h"
#include "resources/FileSystemResourceProvider.h"
#include "resources/ResourceManager.h"
#include "resources/ResourceId.h"

#include <cstdarg>
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using Modern::Vector3;
using Modern::Client::Application;
using Modern::Client::D3D11AssetUploader;
using Modern::Client::D3D11Renderer;
using Modern::Client::DescribeRanMaterialResolution;
using Modern::Client::DisplayMode;
using Modern::Client::FileSystemResourceProvider;
using Modern::Client::IInputSource;
using Modern::Client::ImageAsset;
using Modern::Client::ImageResourceHandle;
using Modern::Client::InputEvent;
using Modern::Client::InputEventType;
using Modern::Client::InputSystem;
using Modern::Client::KeyEvent;
using Modern::Client::KeyCode;
using Modern::Client::MeshAsset;
using Modern::Client::MeshDrawCommand;
using Modern::Client::MeshMaterial;
using Modern::Client::MeshResourceHandle;
using Modern::Client::MeshSection;
using Modern::Client::MeshVertex;
using Modern::Client::MxfMeshTransform;
using Modern::Client::RanMaterialNameKind;
using Modern::Client::RanMeshContainer;
using Modern::Client::RanMeshSource;
using Modern::Client::RenderColor;
using Modern::Client::RendererConfig;
using Modern::Client::ResourceManager;
using Modern::Client::ResourceId;
using Modern::Client::ResourceData;
using Modern::Client::TerrainMeshSet;
using Modern::Client::BuildTerrainMeshes;
using Modern::Result;
using Modern::Status;
using Modern::Status;
using Modern::Client::SkinPiece;
using Modern::Client::SkinPieceContainer;
using Modern::Client::SkinPieceType;
using Modern::Client::TextureCache;
using Modern::Client::TextureLoader;
using Modern::Client::Win32InputSource;
using Modern::Client::Win32PlatformEvents;
using Modern::Client::XMeshDecoder;
using Modern::Client::Gameplay::ClientCharacterController;

namespace
{

// WORLD-PLAYABLE-001B: the matrix helpers now live in rendering/CameraMatrices.h
// so they can be tested without a device and reused by a future Vulkan or
// Metal backend. These bring them into this file.
using ::Modern::Client::BuildIdentityRowMajor;
using ::Modern::Client::BuildLookAtRowMajor;
using ::Modern::Client::BuildPerspectiveRowMajor;
using ::Modern::Client::FrameBox;
using ::Modern::Client::SceneCamera;

constexpr uint32_t kClientWidth  = 960;
constexpr uint32_t kClientHeight = 540;

// The CLIENT-RENDER-002 regression mesh. Still the default so that a plain
// run of the client keeps proving the MXF path works.
constexpr const char* kSkinMeshPath = "data/skin/m_luffy_timeskip.mxf";

// WORLD-PLAYABLE-001C: default character when `--field` is given without
// `--character`. The Luffy regression mesh is a proven, loadable ASURA
// character and doubles as the field-walking avatar.
constexpr const char* kDefaultCharacterMesh = "data/skin/m_luffy_timeskip.mxf";

// WORLD-PLAYABLE-001C: movement speed in world units per second, tuned so a
// brisk walk crosses a reasonable portion of the field in a few seconds.
constexpr float kCharacterMoveSpeed = 250.0f;

// WORLD-PLAYABLE-001C: camera follows the character from this offset, looking
// down at a shallow angle so both the character and the terrain ahead are
// visible.
constexpr Vector3 kCameraFollowOffset(0.0f, 120.0f, -260.0f);

// ASURA-CHARACTER-001B: the directories a RAN wearing file and the mesh it
// names are relative to. Kept as constants so no absolute client path is
// ever compiled in.
constexpr const char* kSkinPieceDir = "data/skinobject";
constexpr const char* kSkinDir      = "data/skin";
constexpr const char* kSkeletonDir  = "data/skeleton";
constexpr const char* kTexturesDir  = "textures";

// The colour drawn for a section whose material names no texture, or whose
// named texture could not be resolved/decoded/uploaded. It is deliberately
// garish so a fallback is unmistakable on screen and in a screenshot, and it is
// never presented as a textured result: every section drawn with it was logged
// as a fallback at setup time.
constexpr RenderColor kFallbackColor(0.85f, 0.05f, 0.55f, 1.0f);

void Log(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	std::vfprintf(stderr, format, args);
	std::fputc('\n', stderr);
	va_end(args);
}

void PrintUsage()
{
	std::printf(
		"ModernClientApp - RAN Online modern client renderer\n"
		"\n"
		"Usage:\n"
		"  ModernClientApp.exe [--asset-root <path>] [--mesh <path>] [--skin-piece <path>]\n"
		"  ModernClientApp.exe --field <name> [--asset-root <path>]\n"
		"  ModernClientApp.exe --audit-textures [--asset-root <path>] [--report <json>]\n"
		"\n"
		"  --asset-root <path>            RAN client tree to load data from\n"
		"                                 (default: $RAN_ASSET_ROOT, then the current directory)\n"
		"  --mesh <path>                  Mesh to render, relative to the asset root\n"
		"                                 (default: data/skin/m_luffy_timeskip.mxf)\n"
		"  --field <name>                 ASURA field to render, read from\n"
		"                                 <root>/data/map/<name>.wld0, e.g. --field bambooforest.\n"
		"                                 Overrides --mesh and --skin-piece when both are given.\n"
		"  --skin-piece <path>            RAN wearing file under data/skinobject to take the\n"
		"                                 mesh from, e.g. data/skinobject/CUN0001_m.cps.\n"
		"                                 Overrides --mesh when both are given.\n"
		"                                 counts, duplicate content and filename collisions, decode\n"
		"                                 a bounded sample, and exit (no window, no GPU)\n"
		"  --report <path>                Also write the full catalog as JSON\n"
		"  --decode-limit <n>             Files decoded per format in the audit (default 300)\n"
		"\n"
		"  The render window is 960x540. Escape or closing the window exits.\n");
}

// ASURA-TEXTURE-001: headless texture inventory. Walks `<root>/textures` and
// catalogues every image candidate from directory metadata (extension, size --
// no full read, so listing the whole tree is cheap even over 14k files) plus
// filename collisions and size-collision groups. It then decodes a bounded
// sample per format through the same content dispatch the renderer uses and
// reports decoded / undecipherable. Deterministic (sorted), windowless and
// device-free. Exhaustive byte-for-byte duplicate detection reads every file
// and is deliberately left out of this fast pass. Returns 0 when the tree was
// walked, non-zero when the textures directory is missing.
int RunTextureAudit(const std::string& assetRoot, const std::string& reportPath,
                    size_t decodeLimitPerFormat)
{
	namespace fs = std::filesystem;
	const fs::path textureRoot = fs::path(assetRoot) / kTexturesDir;
	std::error_code dirEc;
	if (!fs::is_directory(textureRoot, dirEc))
	{
		Log("ModernClientApp: audit found no textures directory under '%s'", assetRoot.c_str());
		return 1;
	}

	struct Tally
	{
		size_t files         = 0;
		size_t bytes         = 0;
		size_t decoded       = 0;
		size_t undecipherable = 0;
	};
	const char* kImageExts[] = { ".dds", ".mtf", ".png", ".tga", ".bmp", ".jpg", ".jpeg" };
	auto isImageExt = [&](const std::string& e) {
		for (const char* t : kImageExts) { if (e == t) { return true; } }
		return false;
	};

	// 1. Catalogue from directory metadata: walk, then sort for determinism.
	struct Entry { std::filesystem::path path; std::string ext; uintmax_t bytes; };
	std::vector<Entry> catalog;
	std::error_code ec;
	for (fs::recursive_directory_iterator it(textureRoot, ec), end; !ec && it != end; it.increment(ec))
	{
		std::error_code fec;
		if (!it->is_regular_file(fec)) { continue; }
		std::string ext = it->path().extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(),
		               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		if (!isImageExt(ext)) { continue; }
		uintmax_t size = it->file_size(fec);
		if (fec) { continue; }
		catalog.push_back({ it->path(), ext, size });
	}
	std::sort(catalog.begin(), catalog.end(),
	          [](const Entry& a, const Entry& b) { return a.path < b.path; });

	std::unordered_map<std::string, Tally>      byExt;
	std::unordered_map<std::string, size_t>     namePathCount; // lower filename -> paths
	std::unordered_map<uintmax_t, size_t>       sizeCount;     // file size -> files
	size_t totalFiles = 0, totalBytes = 0;

	for (const Entry& entry : catalog)
	{
		Tally& tally = byExt[entry.ext];
		++tally.files;
		tally.bytes += static_cast<size_t>(entry.bytes);
		++totalFiles;
		totalBytes += static_cast<size_t>(entry.bytes);

		std::string lowerName = entry.path.filename().string();
		std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
		               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		++namePathCount[lowerName];
		++sizeCount[entry.bytes];
	}
	// A filename collision is one name on more than one path (ambiguous lookup).
	size_t filenameCollisions = 0;
	for (const auto& kv : namePathCount) { if (kv.second > 1) { ++filenameCollisions; } }
	// A duplicate-size group is a byte-size shared by more than one file -- a
	// cheap signal for candidate duplicates without reading every file.
	size_t duplicateSizeGroups = 0;
	for (const auto& kv : sizeCount) { if (kv.second > 1) { ++duplicateSizeGroups; } }

	// 2. Decode a bounded sample per format (reads only the sample).
	std::unordered_map<std::string, size_t> decodedThisExt;
	for (const Entry& entry : catalog)
	{
		if (decodedThisExt[entry.ext] >= decodeLimitPerFormat) { continue; }
		++decodedThisExt[entry.ext];
		std::ifstream file(entry.path, std::ios::binary);
		if (!file) { continue; }
		std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),
		                           std::istreambuf_iterator<char>());
		if (bytes.empty()) { continue; }
		if (TextureLoader::Decode(ResourceData(std::move(bytes))).IsOk()) { ++byExt[entry.ext].decoded; }
		else { ++byExt[entry.ext].undecipherable; }
	}

	std::printf("ModernClientApp: texture audit of '%s'\n", textureRoot.string().c_str());
	std::printf("  %-6s %10s %13s %9s %9s\n",
	            "ext", "files", "bytes", "decoded", "undeciph.");
	for (const char* ext : kImageExts)
	{
		const auto it = byExt.find(ext);
		if (it == byExt.end()) { continue; }
		const Tally& t = it->second;
		std::printf("  %-6s %10zu %13zu %9zu %9zu\n", ext,
		            t.files, t.bytes, t.decoded, t.undecipherable);
	}
	std::printf("  total: %zu files, %zu bytes (%.1f MiB)\n",
	            totalFiles, totalBytes, static_cast<double>(totalBytes) / (1024.0 * 1024.0));
	std::printf("  filename collisions: %zu; duplicate-size groups: %zu "
	            "(decode sample capped at %zu/format)\n",
	            filenameCollisions, duplicateSizeGroups, decodeLimitPerFormat);

	if (!reportPath.empty())
	{
		std::ofstream json(reportPath, std::ios::trunc);
		if (json)
		{
			json << "{\n  \"assetRoot\": \"" << assetRoot << "\",\n";
			json << "  \"totalFiles\": " << totalFiles << ", \"totalBytes\": " << totalBytes << ",\n";
			json << "  \"filenameCollisions\": " << filenameCollisions
			     << ", \"duplicateSizeGroups\": " << duplicateSizeGroups << ",\n";
			json << "  \"formats\": [\n";
			bool first = true;
			for (const char* ext : kImageExts)
			{
				const auto it = byExt.find(ext);
				if (it == byExt.end()) { continue; }
				const Tally& t = it->second;
				if (!first) { json << ",\n"; }
				first = false;
				json << "    { \"ext\": \"" << ext << "\", \"files\": " << t.files
				     << ", \"bytes\": " << t.bytes << ", \"decoded\": " << t.decoded
				     << ", \"undecipherable\": " << t.undecipherable << " }";
			}
			json << "\n  ]\n}\n";
			std::printf("ModernClientApp: wrote inventory JSON to '%s'\n", reportPath.c_str());
		}
		else
		{
			Log("ModernClientApp: could not open report '%s' for writing", reportPath.c_str());
		}
	}
	return 0;
}


// WORLD-PLAYABLE-001B: load one real ASURA field and draw it.
//
// The pipeline is the same one the character path uses, with the terrain
// reader in place of the X decoder:
//
//   .wld0 file -> FileSystemResourceProvider -> WorldStaticMeshReader
//              -> BuildTerrainMeshes -> MeshAsset -> D3D11AssetUploader
//              -> IRenderer::DrawIndexedMesh
//
// No graphics call happens in the parser or in BuildTerrainMeshes; they only
// produce the same CPU-side `MeshAsset` every other mesh in the client already
// produces. Everything D3D-specific is here and in the renderer.
//
// Takes ownership of the platform and input source, exactly as the character
// path does, and returns the process exit code.
int RunField(const std::string& assetRoot,
             const std::string& fieldName,
             std::unique_ptr<Win32PlatformEvents> platform,
             std::unique_ptr<Win32InputSource> inputSource,
             InputSystem& inputSystem,
             D3D11Renderer& renderer,
             D3D11AssetUploader& uploader)
{
    namespace fs = std::filesystem;

    // 1. Locate the field. RAN keeps field geometry beside the field metadata in
    //    `data/map`, as `<name>.wld0` next to `<name>.wld`; the path is built
    //    from the asset root so no client directory is ever compiled in.
    std::string fieldSpec = fieldName;
    if (fieldSpec.size() < 5 || fieldSpec.compare(fieldSpec.size() - 5, 5, ".wld0") != 0)
    {
        fieldSpec += ".wld0";
    }

    // The provider resolves a logical id against the asset root, and a logical id
    // must be a canonical forward-slash relative path -- so the absolute path is
    // only for the size check and for the log, and the id is the relative part.
    const std::string fieldRelative = "data/map/" + fieldSpec;
    const fs::path fieldPath = fs::path(assetRoot) / "data" / "map" / fieldSpec;

    std::error_code sizeEc;
    const uintmax_t fieldBytes = fs::file_size(fieldPath, sizeEc);
    if (sizeEc)
    {
        Log("ModernClientApp: field '%s' was not found (looked at '%s')",
            fieldSpec.c_str(), fieldPath.string().c_str());
        Log("ModernClientApp: a field needs RAN_ASSET_ROOT or --asset-root, and a name that "
            "exists in <root>/data/map, e.g. --field bambooforest");
        return 1;
    }

    Log("ModernClientApp: field '%s' (%llu bytes)",
        fieldPath.string().c_str(), static_cast<unsigned long long>(fieldBytes));

    // Refuse before allocating: the reader's own ceiling is the authority, and
    // a file larger than it is not something this client will try to hold.
    if (fieldBytes == 0 || fieldBytes > ::Modern::Legacy::WorldStaticMeshReader::kMaxInputBytes)
    {
        Log("ModernClientApp: field is %llu bytes, outside the reader's supported range",
            static_cast<unsigned long long>(fieldBytes));
        return 1;
    }

    // 2. Read through the existing resource provider, not a bespoke file read,
    //    so field loading goes through the same path every other asset uses.
    FileSystemResourceProvider resourceProvider(assetRoot);
    if (resourceProvider.Initialize().IsError())
    {
        Log("ModernClientApp: asset root '%s' is not a readable directory", assetRoot.c_str());
        return 1;
    }

    const ::Modern::Result<::Modern::Client::ResourceId> fieldId = ::Modern::Client::ResourceId::Create(fieldRelative);
    if (fieldId.IsError())
    {
        Log("ModernClientApp: field path '%s' is not a usable resource name (%s)",
            fieldRelative.c_str(), fieldId.GetMessage());
        return 1;
    }

    const ::Modern::Result<::Modern::Client::ResourceData> resource = resourceProvider.Load(fieldId.GetValue());
    if (resource.IsError())
    {
        Log("ModernClientApp: could not read field '%s' (%s)",
            fieldPath.string().c_str(), resource.GetMessage());
        return 1;
    }
    const std::vector<uint8_t>& fieldData = resource.GetValue().GetBytes();
    if (fieldData.size() != fieldBytes)
    {
        Log("ModernClientApp: read %zu bytes but the file is %llu bytes",
            fieldData.size(), static_cast<unsigned long long>(fieldBytes));
        return 1;
    }

    // 3. Parse, then convert to renderable meshes.
    Result<::Modern::Legacy::WorldStaticMeshAsset> field =
        ::Modern::Legacy::WorldStaticMeshReader().Decode(fieldData);
    if (field.IsError())
    {
        Log("ModernClientApp: field '%s' was refused by the terrain reader (%s)",
            fieldSpec.c_str(), field.GetMessage());
        Log("ModernClientApp: an encrypted ('Default_Crypt') or truncated .wld0 cannot be "
            "read by this client");
        return 1;
    }

    const ::Modern::Legacy::WorldStaticMeshAsset& parsed = field.GetValue();
    Log("ModernClientApp: parsed %zu mesh(es), %zu leaf record(s), %zu vertices, %zu indices",
        parsed.totalMeshes(), parsed.totalChunks(),
        parsed.totalVertices(), parsed.totalIndices());
    Log("ModernClientApp: tile bounds max=(%.1f, %.1f, %.1f) min=(%.1f, %.1f, %.1f)",
        parsed.tileBoundsMax[0], parsed.tileBoundsMax[1], parsed.tileBoundsMax[2],
        parsed.tileBoundsMin[0], parsed.tileBoundsMin[1], parsed.tileBoundsMin[2]);
    if (parsed.unsupportedFvfCount() != 0)
    {
        for (const ::Modern::Legacy::WorldStaticMeshAsset::UnsupportedFvfEntry& entry : parsed.unsupportedFvfs())
        {
            Log("ModernClientApp: WARNING FVF 0x%03x on %zu leaf/leaves was not modelled",
                entry.fvf, entry.leaves);
        }
    }

    Result<::Modern::Client::TerrainMeshSet> built = BuildTerrainMeshes(parsed);
    if (built.IsError())
    {
        Log("ModernClientApp: field '%s' produced no renderable geometry (%s)",
            fieldSpec.c_str(), built.GetMessage());
        return 1;
    }

    const TerrainMeshSet& terrain = built.GetValue();
    Log("ModernClientApp: %zu renderable mesh(es), %zu vertices, %zu indices",
        terrain.meshes.size(), terrain.totalVertices, terrain.totalIndices);
    Log("ModernClientApp: geometry bounds min=(%.1f, %.1f, %.1f) max=(%.1f, %.1f, %.1f)",
        terrain.boundsMin.x, terrain.boundsMin.y, terrain.boundsMin.z,
        terrain.boundsMax.x, terrain.boundsMax.y, terrain.boundsMax.z);
    for (const TerrainMeshSet::SkipReason& reason : terrain.skipReasons)
    {
        Log("ModernClientApp: %zu leaf/leaves skipped (%s)", reason.count, reason.reason);
    }

    // 4. Upload every leaf. A failed upload is reported and skipped; the field
    //    still renders with whatever reached the GPU, and the exit code says so.
    std::vector<::Modern::Client::MeshResourceHandle> uploaded;
    uploaded.reserve(terrain.meshes.size());
    size_t uploadFailures = 0;
    for (const MeshAsset& mesh : terrain.meshes)
    {
        Result<::Modern::Client::MeshResourceHandle> handle = uploader.UploadMesh(mesh);
        if (handle.IsError() || !handle.GetValue().IsValid())
        {
            ++uploadFailures;
            continue;
        }
        uploaded.push_back(handle.GetValue());
    }

    if (uploaded.empty())
    {
        Log("ModernClientApp: no leaf of '%s' could be uploaded to the GPU", fieldSpec.c_str());
        return 1;
    }
    Log("ModernClientApp: uploaded %zu mesh(es) to the GPU, %zu upload(s) failed",
        uploaded.size(), uploadFailures);

    // 5. A camera that frames the geometry we actually kept.
    //
    //    Terrain reads best from a raised three-quarter angle, so the view
    //    direction is up and back rather than straight on. FrameBox derives the
    //    distance from the bounding sphere so the whole field fits, and the
    //    near/far planes are pushed outside it.
    const SceneCamera camera = FrameBox(terrain.boundsMin, terrain.boundsMax,
                                        Vector3(0.30f, 0.62f, 1.0f));

    float world[16];
    float view[16];
    float projection[16];
    BuildIdentityRowMajor(world);
    BuildLookAtRowMajor(camera.eye, camera.target, camera.up, view);
    BuildPerspectiveRowMajor(camera.fovYRadians,
                             static_cast<float>(kClientWidth) / static_cast<float>(kClientHeight),
                             camera.nearPlane, camera.farPlane, projection);

    Log("ModernClientApp: camera eye=(%.2f, %.2f, %.2f) target=(%.2f, %.2f, %.2f) near=%.2f far=%.2f",
        camera.eye.x, camera.eye.y, camera.eye.z,
        camera.target.x, camera.target.y, camera.target.z,
        camera.nearPlane, camera.farPlane);

    // 6. A single directional light so slopes and facing read as terrain rather
    //    than as flat colour. Terrain has no baked light in this milestone, so
    //    the direction is a constant, not something read from the file.
    const float lightDirection[3] = { -0.45f, -0.80f, -0.40f };
    const float lightColor[3]     = { 1.00f, 0.96f, 0.86f };
    const RenderColor terrainColor(0.42f, 0.52f, 0.33f, 1.0f);
    const RenderColor skyColor(0.55f, 0.72f, 0.92f, 1.0f);

    // 7. The frame loop: same Application, same platform pump, one draw per leaf.
    Application application(std::move(platform));
    application.SetRenderer(&renderer);
    application.SetInputSystem(&inputSystem);
    application.SetInputSource(std::move(inputSource));

    application.SubscribeInput([&application](const InputEvent& event) {
        if (event.type == InputEventType::Key)
        {
            const KeyEvent& key = std::get<KeyEvent>(event.payload);
            if (key.code == KeyCode::Escape && key.pressed)
            {
                application.RequestStop();
            }
        }
    });

    application.SetRenderCallback([&renderer, &uploaded, world, view, projection,
                                  lightDirection, lightColor,
                                  terrainColor, skyColor](uint64_t frameIndex) {
        (void)frameIndex;
        renderer.Clear(skyColor);
        for (const ::Modern::Client::MeshResourceHandle handle : uploaded)
        {
            MeshDrawCommand command;
            command.mesh             = handle;
            command.texture          = ImageResourceHandle::MakeInvalid();
            command.worldMatrix      = world;
            command.viewMatrix       = view;
            command.projectionMatrix = projection;
            command.fallbackColor    = terrainColor;
            command.indexOffset      = 0;
            command.indexCount       = 0;
            command.lightingAmount   = 0.75f;
            for (int i = 0; i < 3; ++i)
            {
                command.lightDirection[i] = lightDirection[i];
                command.lightColor[i]     = lightColor[i];
            }
            renderer.DrawIndexedMesh(command);
        }
    });

    if (application.Initialize().IsError())
    {
        Log("ModernClientApp: application initialization failed");
        for (const ::Modern::Client::MeshResourceHandle handle : uploaded) { uploader.ReleaseMesh(handle); }
        return 1;
    }

    const Status runStatus = application.Run();

    // 8. Teardown in real dependency order: leaf buffers before the device.
    for (const ::Modern::Client::MeshResourceHandle handle : uploaded) { uploader.ReleaseMesh(handle); }
    renderer.Shutdown();

    Log("ModernClientApp: field stopped after %llu frames",
        static_cast<unsigned long long>(application.GetFrameCount()));

    const bool success = runStatus.IsOk() && uploadFailures == 0;
    if (!success)
    {
        Log("ModernClientApp: field run did not complete cleanly");
    }
    return success ? 0 : 1;
}

// WORLD-PLAYABLE-001C: field + character + movement.
//
// Loads a real ASURA field through the WORLD-PLAYABLE-001B pipeline, loads a
// character mesh through the existing character pipeline, places the character
// at the field centre snapped to ground, and runs a frame loop where WASD moves
// the character and the camera follows. Both field and character are rendered
// each frame through real D3D11 draw calls.
//
// The function takes ownership of platform and inputSource, exactly as
// RunField does, and returns the process exit code.
int RunFieldWithCharacter(const std::string& assetRoot,
                          const std::string& fieldName,
                          const std::string& characterMeshPath,
                          std::unique_ptr<Win32PlatformEvents> platform,
                          std::unique_ptr<Win32InputSource> inputSource,
                          InputSystem& inputSystem,
                          D3D11Renderer& renderer,
                          D3D11AssetUploader& uploader)
{
    namespace fs = std::filesystem;

    // ------------------------------------------------------------------
    // 1. Load and parse the field (same pipeline as RunField).
    // ------------------------------------------------------------------
    std::string fieldSpec = fieldName;
    if (fieldSpec.size() < 5 || fieldSpec.compare(fieldSpec.size() - 5, 5, ".wld0") != 0)
    {
        fieldSpec += ".wld0";
    }

    const std::string fieldRelative = "data/map/" + fieldSpec;
    const fs::path fieldPath = fs::path(assetRoot) / "data" / "map" / fieldSpec;

    std::error_code sizeEc;
    const uintmax_t fieldBytes = fs::file_size(fieldPath, sizeEc);
    if (sizeEc)
    {
        Log("ModernClientApp: field '%s' was not found (looked at '%s')",
            fieldSpec.c_str(), fieldPath.string().c_str());
        Log("ModernClientApp: a field needs RAN_ASSET_ROOT or --asset-root, and a name that "
            "exists in <root>/data/map, e.g. --field bambooforest");
        return 1;
    }

    Log("ModernClientApp: field '%s' (%llu bytes)",
        fieldPath.string().c_str(), static_cast<unsigned long long>(fieldBytes));

    if (fieldBytes == 0 || fieldBytes > ::Modern::Legacy::WorldStaticMeshReader::kMaxInputBytes)
    {
        Log("ModernClientApp: field is %llu bytes, outside the reader's supported range",
            static_cast<unsigned long long>(fieldBytes));
        return 1;
    }

    FileSystemResourceProvider resourceProvider(assetRoot);
    if (resourceProvider.Initialize().IsError())
    {
        Log("ModernClientApp: asset root '%s' is not a readable directory", assetRoot.c_str());
        return 1;
    }

    const ::Modern::Result<::Modern::Client::ResourceId> fieldId =
        ::Modern::Client::ResourceId::Create(fieldRelative);
    if (fieldId.IsError())
    {
        Log("ModernClientApp: field path '%s' is not a usable resource name (%s)",
            fieldRelative.c_str(), fieldId.GetMessage());
        return 1;
    }

    const ::Modern::Result<::Modern::Client::ResourceData> resource =
        resourceProvider.Load(fieldId.GetValue());
    if (resource.IsError())
    {
        Log("ModernClientApp: could not read field '%s' (%s)",
            fieldPath.string().c_str(), resource.GetMessage());
        return 1;
    }
    const std::vector<uint8_t>& fieldData = resource.GetValue().GetBytes();
    if (fieldData.size() != fieldBytes)
    {
        Log("ModernClientApp: read %zu bytes but the file is %llu bytes",
            fieldData.size(), static_cast<unsigned long long>(fieldBytes));
        return 1;
    }

    Result<::Modern::Legacy::WorldStaticMeshAsset> field =
        ::Modern::Legacy::WorldStaticMeshReader().Decode(fieldData);
    if (field.IsError())
    {
        Log("ModernClientApp: field '%s' was refused by the terrain reader (%s)",
            fieldSpec.c_str(), field.GetMessage());
        Log("ModernClientApp: an encrypted ('Default_Crypt') or truncated .wld0 cannot be "
            "read by this client");
        return 1;
    }

    const ::Modern::Legacy::WorldStaticMeshAsset& parsed = field.GetValue();
    Log("ModernClientApp: parsed %zu mesh(es), %zu leaf record(s), %zu vertices, %zu indices",
        parsed.totalMeshes(), parsed.totalChunks(),
        parsed.totalVertices(), parsed.totalIndices());

    Result<::Modern::Client::TerrainMeshSet> built = BuildTerrainMeshes(parsed);
    if (built.IsError())
    {
        Log("ModernClientApp: field '%s' produced no renderable geometry (%s)",
            fieldSpec.c_str(), built.GetMessage());
        return 1;
    }

    const TerrainMeshSet& terrain = built.GetValue();
    Log("ModernClientApp: %zu renderable mesh(es), %zu vertices, %zu indices",
        terrain.meshes.size(), terrain.totalVertices, terrain.totalIndices);
    Log("ModernClientApp: geometry bounds min=(%.1f, %.1f, %.1f) max=(%.1f, %.1f, %.1f)",
        terrain.boundsMin.x, terrain.boundsMin.y, terrain.boundsMin.z,
        terrain.boundsMax.x, terrain.boundsMax.y, terrain.boundsMax.z);
    for (const TerrainMeshSet::SkipReason& reason : terrain.skipReasons)
    {
        Log("ModernClientApp: %zu leaf/leaves skipped (%s)", reason.count, reason.reason);
    }

    // ------------------------------------------------------------------
    // 2. Upload field meshes to GPU.
    // ------------------------------------------------------------------
    std::vector<::Modern::Client::MeshResourceHandle> fieldHandles;
    fieldHandles.reserve(terrain.meshes.size());
    size_t fieldUploadFailures = 0;
    for (const MeshAsset& mesh : terrain.meshes)
    {
        Result<::Modern::Client::MeshResourceHandle> handle = uploader.UploadMesh(mesh);
        if (handle.IsError() || !handle.GetValue().IsValid())
        {
            ++fieldUploadFailures;
            continue;
        }
        fieldHandles.push_back(handle.GetValue());
    }

    if (fieldHandles.empty())
    {
        Log("ModernClientApp: no leaf of '%s' could be uploaded to the GPU", fieldSpec.c_str());
        return 1;
    }
    Log("ModernClientApp: uploaded %zu field mesh(es) to the GPU, %zu upload(s) failed",
        fieldHandles.size(), fieldUploadFailures);

    // ------------------------------------------------------------------
    // 3. Load character mesh through the existing character pipeline.
    // ------------------------------------------------------------------
    Log("ModernClientApp: rendering character mesh '%s'", characterMeshPath.c_str());

    const Modern::Result<ResourceId> charMeshId = ResourceId::Create(characterMeshPath);
    if (charMeshId.IsError())
    {
        Log("ModernClientApp: bad resource id '%s' (%s)", characterMeshPath.c_str(),
            charMeshId.GetMessage());
        return 1;
    }

    const Modern::Result<ResourceData> charFileData = resourceProvider.Load(charMeshId.GetValue());
    if (charFileData.IsError())
    {
        Log("ModernClientApp: failed to load '%s' (%s) -- is the asset root a RAN client tree?",
            characterMeshPath.c_str(), charFileData.GetMessage());
        return 1;
    }

    const Modern::Result<RanMeshContainer> container = RanMeshSource::Classify(charFileData.GetValue());
    if (container.IsError())
    {
        Log("ModernClientApp: '%s' is neither a plain .X nor a readable .mxf (%s)",
            characterMeshPath.c_str(), container.GetMessage());
        return 1;
    }
    Log("ModernClientApp: character mesh container is %s", ToString(container.GetValue()));

    const Modern::Result<ResourceData> xFileData = RanMeshSource::ToPlainX(charFileData.GetValue());
    if (xFileData.IsError())
    {
        Log("ModernClientApp: container transform of '%s' failed (%s)",
            characterMeshPath.c_str(), xFileData.GetMessage());
        return 1;
    }

    XMeshDecoder xMeshDecoder;
    const Modern::Result<MeshAsset> charMesh = xMeshDecoder.DecodeMesh(xFileData.GetValue());
    if (charMesh.IsError())
    {
        Log("ModernClientApp: X mesh decode of '%s' failed (%s)",
            characterMeshPath.c_str(), charMesh.GetMessage());
        return 1;
    }

    const MeshAsset& characterMeshAsset = charMesh.GetValue();
    Log("ModernClientApp: decoded %s: %zu vertices, %zu indices, %zu triangles",
        characterMeshPath.c_str(), characterMeshAsset.GetVertexCount(),
        characterMeshAsset.GetIndexCount(), characterMeshAsset.GetTriangleCount());

    const Modern::Result<::Modern::Client::MeshResourceHandle> charHandle =
        uploader.UploadMesh(characterMeshAsset);
    if (charHandle.IsError())
    {
        Log("ModernClientApp: GPU upload of character mesh failed (%s)",
            charHandle.GetMessage());
        return 1;
    }
    const MeshResourceHandle characterMesh = charHandle.GetValue();

    // Character materials and textures.
    const std::vector<MeshMaterial>& charMaterials = characterMeshAsset.GetMaterials();
    const std::vector<MeshSection>&   charSections  = characterMeshAsset.GetSections();

    std::vector<ImageResourceHandle> charMaterialTextures(charMaterials.size(),
                                                          ImageResourceHandle::MakeInvalid());
    std::vector<ImageResourceHandle> uploadedCharTextures;
    bool charTexturesAllLoaded = true;

    Log("ModernClientApp: character mesh carries %zu materials and %zu sections",
        charMaterials.size(), charSections.size());

    if (characterMeshAsset.HasSections())
    {
        const std::filesystem::path textureRoot = std::filesystem::path(assetRoot) / kTexturesDir;
        TextureLoader textureLoader;
        const Status textureInit = textureLoader.Initialize(textureRoot);
        if (textureInit.IsError())
        {
            Log("ModernClientApp: texture tree '%s' could not be indexed (%s)",
                textureRoot.string().c_str(), textureInit.GetMessage());
            charTexturesAllLoaded = false;
        }
        else
        {
            TextureCache textureCache;
            std::unordered_map<std::string, ImageResourceHandle> textureUploadCache;

            for (size_t m = 0; m < charMaterials.size(); ++m)
            {
                const MeshMaterial& material = charMaterials[m];
                if (material.textureFilename.empty())
                {
                    Log("ModernClientApp: material '%s' declares no texture; it draws the fallback color by design",
                        material.name.c_str());
                    continue;
                }

                const RanMaterialNameKind nameKind =
                    Modern::Client::ClassifyRanMaterialName(material.textureFilename);
                if (nameKind == RanMaterialNameKind::RanMaterialSlot)
                {
                    Log("ModernClientApp: %s",
                        DescribeRanMaterialResolution(std::string(), material.textureFilename,
                                                     "").c_str());
                    charTexturesAllLoaded = false;
                    continue;
                }

                const std::filesystem::path resolved = textureLoader.Find(material.textureFilename);
                if (resolved.empty())
                {
                    Log("ModernClientApp: %s (searched tree '%s')",
                        DescribeRanMaterialResolution(std::string(), material.textureFilename,
                                                     "").c_str(),
                        textureRoot.string().c_str());
                    charTexturesAllLoaded = false;
                    continue;
                }

                const std::string key = resolved.lexically_normal().string();
                const auto alreadyUploaded = textureUploadCache.find(key);
                if (alreadyUploaded != textureUploadCache.end())
                {
                    charMaterialTextures[m] = alreadyUploaded->second;
                    continue;
                }

                const Modern::Result<const ImageAsset*> image = textureCache.Acquire(resolved);
                if (image.IsError())
                {
                    Log("ModernClientApp: material '%s' texture '%s' (%s) failed to decode (%s)",
                        material.name.c_str(), material.textureFilename.c_str(),
                        resolved.string().c_str(), image.GetMessage());
                    charTexturesAllLoaded = false;
                    continue;
                }

                const Modern::Result<ImageResourceHandle> uploaded =
                    uploader.UploadImage(*image.GetValue());
                if (uploaded.IsError())
                {
                    Log("ModernClientApp: material '%s' texture '%s' (%s) failed to upload (%s)",
                        material.name.c_str(), material.textureFilename.c_str(),
                        resolved.string().c_str(), uploaded.GetMessage());
                    charTexturesAllLoaded = false;
                    continue;
                }

                charMaterialTextures[m] = uploaded.GetValue();
                textureUploadCache.emplace(key, uploaded.GetValue());
                uploadedCharTextures.push_back(uploaded.GetValue());
                Log("ModernClientApp: material '%s' texture '%s' -> '%s' uploaded as image %llu "
                    "(%u x %u, %s)",
                    material.name.c_str(), material.textureFilename.c_str(),
                    resolved.string().c_str(),
                    static_cast<unsigned long long>(uploaded.GetValue().Get()),
                    image.GetValue()->GetWidth(), image.GetValue()->GetHeight(),
                    ToString(image.GetValue()->GetFormat()));
            }

            Log("ModernClientApp: character texture cache holds %zu decoded image(s); %zu uploaded to the GPU",
                textureCache.GetCachedCount(), textureUploadCache.size());
        }

        for (size_t s = 0; s < charSections.size(); ++s)
        {
            const MeshSection& section = charSections[s];
            const bool textured = section.materialIndex < charMaterialTextures.size() &&
                                  charMaterialTextures[section.materialIndex].IsValid();
            const char* binding = "none (fallback color)";
            if (textured)
            {
                binding = std::to_string(charMaterialTextures[section.materialIndex].Get()).c_str();
            }
            Log("ModernClientApp: character section %zu: indices [%u, %u) material %zu texture -> %s",
                s, section.indexOffset, section.indexOffset + section.indexCount,
                section.materialIndex, binding);
        }
    }

    // ------------------------------------------------------------------
    // 4. Place character at field centre, snapped to ground.
    // ------------------------------------------------------------------
    ClientCharacterController controller;
    controller.moveSpeed = kCharacterMoveSpeed;

    const Vector3 fieldCentre = (terrain.boundsMin + terrain.boundsMax) * 0.5f;
    const std::optional<float> spawnGroundY = QueryGroundHeight(terrain, fieldCentre.x, fieldCentre.z);
    float spawnY = spawnGroundY.has_value() ? *spawnGroundY + controller.groundOffset
                                            : (terrain.boundsMin.y + terrain.boundsMax.y) * 0.5f;
    controller.position = Vector3(fieldCentre.x, spawnY, fieldCentre.z);

    Log("ModernClientApp: character spawn at (%.2f, %.2f, %.2f), groundY=%s",
        controller.position.x, controller.position.y, controller.position.z,
        spawnGroundY.has_value() ? std::to_string(*spawnGroundY).c_str() : "none (fallback)");

    // ------------------------------------------------------------------
    // 5. Camera follows character from behind and above.
    // ------------------------------------------------------------------
    float world[16] = {};
    float view[16] = {};
    float projection[16] = {};
    BuildIdentityRowMajor(world);

    constexpr float kFovY = 60.0f * 3.14159265358979323846f / 180.0f;
    const float aspect = static_cast<float>(kClientWidth) / static_cast<float>(kClientHeight);
    const float nearPlane = 0.1f;
    const float farPlane = 2000.0f;
    BuildPerspectiveRowMajor(kFovY, aspect, nearPlane, farPlane, projection);

    const float lightDirection[3] = { -0.45f, -0.80f, -0.40f };
    const float lightColor[3]     = { 1.00f, 0.96f, 0.86f };
    const RenderColor terrainColor(0.42f, 0.52f, 0.33f, 1.0f);
    const RenderColor skyColor(0.55f, 0.72f, 0.92f, 1.0f);
    const RenderColor characterFallbackColor(0.85f, 0.05f, 0.55f, 1.0f);

    // ------------------------------------------------------------------
    // 6. Frame loop: input → movement → draw field + character.
    // ------------------------------------------------------------------
    Application application(std::move(platform));
    application.SetRenderer(&renderer);
    application.SetInputSystem(&inputSystem);
    application.SetInputSource(std::move(inputSource));

    // WASD movement + Escape to quit.
    bool keyW = false, keyA = false, keyS = false, keyD = false;
    application.SubscribeInput([&application, &keyW, &keyA, &keyS, &keyD, &controller]
                               (const InputEvent& event) {
        if (event.type != InputEventType::Key) { return; }
        const KeyEvent& key = std::get<KeyEvent>(event.payload);
        bool* target = nullptr;
        switch (key.code)
        {
            case KeyCode::W:    target = &keyW;    break;
            case KeyCode::A:    target = &keyA;    break;
            case KeyCode::S:    target = &keyS;    break;
            case KeyCode::D:    target = &keyD;    break;
            case KeyCode::Escape: application.RequestStop(); return;
            default: return;
        }
        *target = key.pressed;
    });

    application.SetRenderCallback([&renderer, &fieldHandles, &terrain, &controller,
                                   &characterMesh, &charSections, &charMaterialTextures,
                                   &uploadedCharTextures,
                                   &world, &view, &projection,
                                   lightDirection, lightColor,
                                   terrainColor, skyColor, characterFallbackColor,
                                   &keyW, &keyA, &keyS, &keyD](uint64_t frameIndex) {
        (void)frameIndex;

        // --- Input → movement direction (XZ plane) ---
        Vector3 moveDir = Vector3::Zero;
        if (keyW) moveDir.z += 1.0f;
        if (keyS) moveDir.z -= 1.0f;
        if (keyD) moveDir.x += 1.0f;
        if (keyA) moveDir.x -= 1.0f;
        controller.SetMoveDirection(moveDir);

        // --- Update character (dt = elapsed seconds, not frame count) ---
        // The Application loop calls the render callback once per frame; we
        // assume roughly constant dt for movement. A future milestone can wire
        // a real timer here.
        constexpr float kDt = 1.0f / 60.0f;
        controller.Update(kDt,
            [&terrain](float x, float z) { return QueryGroundHeight(terrain, x, z); },
            terrain.boundsMin, terrain.boundsMax);

        // --- Camera follows character ---
        const Vector3 camEye = controller.position + kCameraFollowOffset;
        BuildLookAtRowMajor(camEye, controller.position, Vector3::Up, view);

        renderer.Clear(skyColor);

        // Draw field meshes (one draw per leaf, no texture, terrain colour).
        for (const ::Modern::Client::MeshResourceHandle handle : fieldHandles)
        {
            MeshDrawCommand command;
            command.mesh             = handle;
            command.texture          = ImageResourceHandle::MakeInvalid();
            command.worldMatrix      = world;
            command.viewMatrix       = view;
            command.projectionMatrix = projection;
            command.fallbackColor    = terrainColor;
            command.indexOffset      = 0;
            command.indexCount       = 0;
            command.lightingAmount   = 0.75f;
            for (int i = 0; i < 3; ++i)
            {
                command.lightDirection[i] = lightDirection[i];
                command.lightColor[i]     = lightColor[i];
            }
            renderer.DrawIndexedMesh(command);
        }

        // Draw character: one indexed draw per section, each with its own texture.
        float charWorld[16] = {};
        // Translation matrix: move character to its world position.
        charWorld[0] = charWorld[5] = charWorld[10] = charWorld[15] = 1.0f;
        charWorld[12] = controller.position.x;
        charWorld[13] = controller.position.y;
        charWorld[14] = controller.position.z;

        for (const MeshSection& section : charSections)
        {
            const bool hasTexture = section.materialIndex < charMaterialTextures.size() &&
                                    charMaterialTextures[section.materialIndex].IsValid();

            MeshDrawCommand command;
            command.mesh             = characterMesh;
            command.texture          = hasTexture
                ? charMaterialTextures[section.materialIndex]
                : ImageResourceHandle::MakeInvalid();
            command.worldMatrix      = charWorld;
            command.viewMatrix       = view;
            command.projectionMatrix = projection;
            command.indexOffset      = section.indexOffset;
            command.indexCount       = section.indexCount;
            command.fallbackColor    = hasTexture ? RenderColor::White : characterFallbackColor;
            command.lightingAmount   = 0.75f;
            for (int i = 0; i < 3; ++i)
            {
                command.lightDirection[i] = lightDirection[i];
                command.lightColor[i]     = lightColor[i];
            }
            renderer.DrawIndexedMesh(command);
        }
    });

    if (application.Initialize().IsError())
    {
        Log("ModernClientApp: application initialization failed");
        for (const ::Modern::Client::MeshResourceHandle handle : fieldHandles) { uploader.ReleaseMesh(handle); }
        uploader.ReleaseMesh(characterMesh);
        return 1;
    }

    const Status runStatus = application.Run();

    // Teardown in real dependency order.
    for (const ImageResourceHandle texture : uploadedCharTextures)
    {
        uploader.ReleaseImage(texture);
    }
    uploader.ReleaseMesh(characterMesh);
    for (const ::Modern::Client::MeshResourceHandle handle : fieldHandles) { uploader.ReleaseMesh(handle); }
    renderer.Shutdown();

    Log("ModernClientApp: field+character stopped after %llu frames",
        static_cast<unsigned long long>(application.GetFrameCount()));

    const bool success = runStatus.IsOk() && fieldUploadFailures == 0;
    if (!success)
    {
        Log("ModernClientApp: field+character run did not complete cleanly");
    }
    return success ? 0 : 1;
}

} // namespace


int main(int argc, char** argv)
{
	std::string assetRoot;
	std::string meshPath;
	std::string skinPiecePath;
	std::string characterMeshPath;
	std::string fieldName;
	bool        auditTextures = false;
	std::string reportPath;
	size_t      decodeLimit = 300;
	for (int i = 1; i < argc; ++i)
	{
		const std::string argument = argv[i];
		if (argument == "--asset-root" && i + 1 < argc)
		{
			assetRoot = argv[++i];
		}
		else if (argument.rfind("--asset-root=", 0) == 0)
		{
			assetRoot = argument.substr(std::strlen("--asset-root="));
		}
		else if (argument == "--mesh" && i + 1 < argc)
		{
			meshPath = argv[++i];
		}
		else if (argument.rfind("--mesh=", 0) == 0)
		{
			meshPath = argument.substr(std::strlen("--mesh="));
		}
		else if (argument == "--skin-piece" && i + 1 < argc)
		{
			skinPiecePath = argv[++i];
		}
		else if (argument.rfind("--skin-piece=", 0) == 0)
		{
			skinPiecePath = argument.substr(std::strlen("--skin-piece="));
		}
		else if (argument == "--character" && i + 1 < argc)
		{
			characterMeshPath = argv[++i];
		}
		else if (argument.rfind("--character=", 0) == 0)
		{
			characterMeshPath = argument.substr(std::strlen("--character="));
		}
		else if (argument == "--field" && i + 1 < argc)
		{
			fieldName = argv[++i];
		}
		else if (argument.rfind("--field=", 0) == 0)
		{
			fieldName = argument.substr(std::strlen("--field="));
		}
		else if (argument == "--audit-textures")
		{
			auditTextures = true;
		}
		else if (argument == "--report" && i + 1 < argc)
		{
			reportPath = argv[++i];
		}
		else if (argument == "--decode-limit" && i + 1 < argc)
		{
			decodeLimit = static_cast<size_t>(std::strtoull(argv[++i], nullptr, 10));
		}
		else if (argument == "--help" || argument == "-h")
		{
			PrintUsage();
			return 0;
		}
		else
		{
			Log("ModernClientApp: unknown argument '%s'", argument.c_str());
			PrintUsage();
			return 2;
		}
	}

	if (assetRoot.empty())
	{
		if (const char* environmentRoot = std::getenv("RAN_ASSET_ROOT"))
		{
			assetRoot = environmentRoot;
		}
		else
		{
			assetRoot = ".";
		}
	}

	Log("ModernClientApp: asset root '%s'", assetRoot.c_str());

	// Headless texture inventory: no window, no device. This is the batch
	// validation path that walks the whole tree (a frame-bound window would
	// not), so it runs before anything GPU is created.
	if (auditTextures)
	{
		return RunTextureAudit(assetRoot, reportPath, decodeLimit);
	}

	// 1. Platform window. Everything else binds to this window handle.
	//
	//    The platform is heap-owned and MOVED into the Application below, so
	//    the loop that pumps messages is the same object that created the
	//    window. Owning it on the stack instead leaves the Application running
	//    its default NullPlatformEvents, whose PumpEvents() never touches
	//    PeekMessageW: the window is created and drawn to, but no Windows
	//    message is ever dispatched, so WM_CLOSE never reaches the window
	//    procedure and the process can only be killed.
	auto platform = std::make_unique<Win32PlatformEvents>("RAN Online Modern", kClientWidth, kClientHeight);
	if (!platform->Initialize())
	{
		Log("ModernClientApp: window initialization failed");
		return 1;
	}

	// 2. Input: Win32 samples translated to platform-independent events.
	auto inputSource = std::make_unique<Win32InputSource>(platform.get());
	platform->SetInputSource(inputSource.get());

	InputSystem inputSystem;
	const Status inputInitStatus = inputSystem.Initialize();
	if (inputInitStatus.IsError())
	{
		Log("ModernClientApp: input system initialization failed (%s)",
		    inputInitStatus.GetMessage());
		return 1;
	}

	// 3. Renderer: real Direct3D 11 device on the native window.
	D3D11Renderer renderer;
	RendererConfig rendererConfig;
	rendererConfig.width         = kClientWidth;
	rendererConfig.height        = kClientHeight;
	rendererConfig.displayMode   = DisplayMode::Windowed;
	rendererConfig.vsync         = true;
	rendererConfig.windowHandle  = platform->GetWindowHandle();

	if (renderer.Initialize(rendererConfig).IsError())
	{
		Log("ModernClientApp: renderer initialization failed -- no D3D11 device on this machine");
		return 1;
	}

	// 4. Asset uploader wired to the renderer's device and context.
	D3D11AssetUploader uploader(&renderer);
	renderer.SetAssetUploader(&uploader);

	// WORLD-PLAYABLE-001C: a field with a character. When `--field` and
	// `--character` (or just `--field` with the default character) are both
	// given, run the combined field + character + movement loop. Otherwise
	// fall through to the original field-only behaviour.
	if (!fieldName.empty())
	{
		const std::string characterPath =
		    characterMeshPath.empty() ? std::string(kDefaultCharacterMesh) : characterMeshPath;
		return RunFieldWithCharacter(assetRoot, fieldName, characterPath,
		                             std::move(platform), std::move(inputSource),
		                             inputSystem, renderer, uploader);
	}

	// 5. The character asset through the real pipeline:
	//    file -> ResourceManager -> container -> X decoder -> MeshAsset.
	FileSystemResourceProvider resourceProvider(assetRoot);
	if (resourceProvider.Initialize().IsError())
	{
		Log("ModernClientApp: asset root '%s' is not a readable directory", assetRoot.c_str());
		return 1;
	}

	ResourceManager resourceManager;
	resourceManager.SetProvider(&resourceProvider);
	if (resourceManager.Initialize().IsError())
	{
		Log("ModernClientApp: resource manager initialization failed");
		return 1;
	}

	// 5a. Mesh selection.
	//
	//     Default: the CLIENT-RENDER-002 regression `.mxf`.
	//     --mesh:  an explicit path relative to the asset root.
	//     --skin-piece: a RAN wearing file; its mesh field names the mesh, so
	//     the character is reached through the real RAN item -> piece -> mesh
	//     chain rather than by naming the mesh by hand.
	//
	//     --skin-piece wins over --mesh, because it carries strictly more
	//     information about which character is meant.
	std::string selectedMeshPath = meshPath.empty() ? std::string(kSkinMeshPath) : meshPath;
	std::string pieceSource;   // what to attribute materials to in diagnostics
	SkinPiece   piece;

	if (!skinPiecePath.empty())
	{
		const Modern::Result<ResourceId> pieceId = ResourceId::Create(skinPiecePath);
		if (pieceId.IsError())
		{
			Log("ModernClientApp: bad skin-piece resource id '%s' (%s)",
			    skinPiecePath.c_str(), pieceId.GetMessage());
			return 1;
		}
		const Modern::Result<ResourceData> pieceData =
		    resourceManager.Load(pieceId.GetValue());
		if (pieceData.IsError())
		{
			Log("ModernClientApp: failed to load skin piece '%s' (%s)",
			    skinPiecePath.c_str(), pieceData.GetMessage());
			return 1;
		}
		const Modern::Result<SkinPiece> decodedPiece =
		    SkinPieceContainer::Decode(pieceData.GetValue());
		if (decodedPiece.IsError())
		{
			Log("ModernClientApp: skin piece '%s' could not be decoded (%s)",
			    skinPiecePath.c_str(), decodedPiece.GetMessage());
			return 1;
		}
		piece = decodedPiece.GetValue();
		pieceSource = skinPiecePath;

		Log("ModernClientApp: RAN wearing piece %s: version 0x%04X, type %s, "
		    "mesh '%s', skeleton '%s', mesh node '%s', %u material slot(s), "
		    "%zu byte(s) of undecoded tail retained",
		    skinPiecePath.c_str(), piece.version, ToString(piece.GetPieceType()),
		    piece.meshFilename.c_str(), piece.skeletonFilename.c_str(),
		    piece.meshNodeName.c_str(), piece.materialSlotCount,
		    piece.GetOpaqueTailSize());

		// The piece's mesh lives under data/skin and its skeleton under
		// data/skeleton. Both are relative to the asset root, so no absolute
		// client path is ever compiled in.
		selectedMeshPath = std::string(kSkinDir) + "/" + piece.meshFilename;

		const std::filesystem::path skeletonPath =
		    std::filesystem::path(assetRoot) / kSkeletonDir / piece.skeletonFilename;
		Log("ModernClientApp: piece skeleton '%s' %s",
		    piece.skeletonFilename.c_str(),
		    std::filesystem::is_regular_file(skeletonPath) ? "present" : "MISSING");
	}

	Log("ModernClientApp: rendering mesh '%s'", selectedMeshPath.c_str());

	const Modern::Result<ResourceId> skinMeshId = ResourceId::Create(selectedMeshPath);
	if (skinMeshId.IsError())
	{
		Log("ModernClientApp: bad resource id '%s' (%s)", selectedMeshPath.c_str(),
		    skinMeshId.GetMessage());
		return 1;
	}

	const Modern::Result<ResourceData> skinFileData = resourceManager.Load(skinMeshId.GetValue());
	if (skinFileData.IsError())
	{
		Log("ModernClientApp: failed to load '%s' (%s) -- is the asset root a RAN client tree?",
		    selectedMeshPath.c_str(), skinFileData.GetMessage());
		return 1;
	}

	// 5b. Container routing. RAN ships the same DirectX `.x` content either
	//     obfuscated in an `.mxf` or as a plain `.X`; which one this is is
	//     decided from the bytes, not from the extension.
	const Modern::Result<RanMeshContainer> container = RanMeshSource::Classify(skinFileData.GetValue());
	if (container.IsError())
	{
		Log("ModernClientApp: '%s' is neither a plain .X nor a readable .mxf (%s)",
		    selectedMeshPath.c_str(), container.GetMessage());
		return 1;
	}
	Log("ModernClientApp: mesh container is %s", ToString(container.GetValue()));

	const Modern::Result<ResourceData> xFileData = RanMeshSource::ToPlainX(skinFileData.GetValue());
	if (xFileData.IsError())
	{
		Log("ModernClientApp: container transform of '%s' failed (%s)",
		    selectedMeshPath.c_str(), xFileData.GetMessage());
		return 1;
	}

	XMeshDecoder xMeshDecoder;
	const Modern::Result<MeshAsset> skinMesh = xMeshDecoder.DecodeMesh(xFileData.GetValue());
	if (skinMesh.IsError())
	{
		Log("ModernClientApp: X mesh decode of '%s' failed (%s)",
		    selectedMeshPath.c_str(), skinMesh.GetMessage());
		return 1;
	}

	const MeshAsset& mesh = skinMesh.GetValue();
	Log("ModernClientApp: decoded %s: %zu vertices, %zu indices, %zu triangles",
	    selectedMeshPath.c_str(), mesh.GetVertexCount(), mesh.GetIndexCount(),
	    mesh.GetTriangleCount());

	// 6. Upload the mesh to real GPU buffers.
	const Modern::Result<::Modern::Client::MeshResourceHandle> meshHandle = uploader.UploadMesh(mesh);
	if (meshHandle.IsError())
	{
		Log("ModernClientApp: GPU upload of the character mesh failed (%s)",
		    meshHandle.GetMessage());
		return 1;
	}

	const MeshResourceHandle characterMesh = meshHandle.GetValue();

	// 6b. CLIENT-RENDER-002: the character's real materials and textures.
	//
	//     The decoded mesh names its materials and, per MeshSection, which
	//     material each run of indices is drawn with. Each material names one
	//     texture by bare filename; that filename is resolved over the ASURA
	//     texture tree by TextureLoader (the same name/path resolution the
	//     legacy TextureManager performed, with no D3D coupling) and decoded by
	//     content magic -- PNG for the Luffy regression asset, DDS for the RAN
	//     body, both through the same dispatch.
	//
	//     ASURA-CHARACTER-001B adds one distinction that matters here. RAN
	//     meshes also carry RAN-internal material slot names such as IMAGE01
	//     or BLOOD01, which are role identifiers rather than filenames and
	//     which no shipped texture is named after. When one of those reaches
	//     the loader it is reported as an unresolved RAN slot -- naming the
	//     piece, the material and the attempt -- rather than as a generic
	//     missing file, and it is never counted as a textured draw.
	//
	//     Every *unique* texture is decoded and uploaded exactly once and the
	//     handle is shared by every material that names it, so a mesh that
	//     references one body texture from sixteen materials uploads it once.
	//     A material that names no texture, or whose named texture cannot be
	//     resolved, decoded or uploaded, is left with an invalid handle and a
	//     clearly-labelled fallback: that is reported, never silently drawn as
	//     a success.
	const std::vector<MeshMaterial>& materials = mesh.GetMaterials();
	const std::vector<MeshSection>&   sections  = mesh.GetSections();

	std::vector<ImageResourceHandle> materialTextures(materials.size(),
	                                                  ImageResourceHandle::MakeInvalid());
	std::vector<ImageResourceHandle> uploadedTextures;  // unique handles, for teardown
	bool namedTexturesAllLoaded = true;

	Log("ModernClientApp: mesh carries %zu materials and %zu sections",
	    materials.size(), sections.size());

	if (mesh.HasSections())
	{
		const std::filesystem::path textureRoot = std::filesystem::path(assetRoot) / kTexturesDir;
		TextureLoader textureLoader;
		const Status textureInit = textureLoader.Initialize(textureRoot);
		if (textureInit.IsError())
		{
			Log("ModernClientApp: texture tree '%s' could not be indexed (%s)",
			    textureRoot.string().c_str(), textureInit.GetMessage());
			namedTexturesAllLoaded = false;
		}
		else
		{
			// Decode-once (TextureCache) and upload-once (textureUploadCache),
			// both keyed by the resolved file, so a texture referenced by many
			// materials -- or by different names that resolve to one file -- is
			// read, decoded and uploaded exactly once rather than per reference.
			TextureCache textureCache;
			std::unordered_map<std::string, ImageResourceHandle> textureUploadCache;

			for (size_t m = 0; m < materials.size(); ++m)
			{
				const MeshMaterial& material = materials[m];
				if (material.textureFilename.empty())
				{
					// A material with no texture is a real, legal shape; it draws
					// with the fallback color by design, which is not a failure.
					Log("ModernClientApp: material '%s' declares no texture; it draws the fallback color by design",
					    material.name.c_str());
					continue;
				}

				// ASURA-CHARACTER-001B: a RAN-internal slot name is reported as
				// such, up front, instead of as a bare-name lookup miss.
				const RanMaterialNameKind nameKind =
				    Modern::Client::ClassifyRanMaterialName(material.textureFilename);
				if (nameKind == RanMaterialNameKind::RanMaterialSlot)
				{
					Log("ModernClientApp: %s",
					    DescribeRanMaterialResolution(pieceSource, material.textureFilename,
					                                 "").c_str());
					namedTexturesAllLoaded = false;
					continue;
				}

				const std::filesystem::path resolved = textureLoader.Find(material.textureFilename);
				if (resolved.empty())
				{
					Log("ModernClientApp: %s (searched tree '%s')",
					    DescribeRanMaterialResolution(pieceSource, material.textureFilename,
					                                 "").c_str(),
					    textureRoot.string().c_str());
					namedTexturesAllLoaded = false;
					continue;
				}

				const std::string key = resolved.lexically_normal().string();
				// Already decoded and uploaded under this resolved path: bind the
				// same handle again, touching neither the file nor the GPU.
				const auto alreadyUploaded = textureUploadCache.find(key);
				if (alreadyUploaded != textureUploadCache.end())
				{
					materialTextures[m] = alreadyUploaded->second;
					continue;
				}

				const Modern::Result<const ImageAsset*> image = textureCache.Acquire(resolved);
				if (image.IsError())
				{
					Log("ModernClientApp: material '%s' texture '%s' (%s) failed to decode (%s)",
					    material.name.c_str(), material.textureFilename.c_str(),
					    resolved.string().c_str(), image.GetMessage());
					namedTexturesAllLoaded = false;
					continue;
				}

				const Modern::Result<ImageResourceHandle> uploaded =
				    uploader.UploadImage(*image.GetValue());
				if (uploaded.IsError())
				{
					Log("ModernClientApp: material '%s' texture '%s' (%s) failed to upload (%s)",
					    material.name.c_str(), material.textureFilename.c_str(),
					    resolved.string().c_str(), uploaded.GetMessage());
					namedTexturesAllLoaded = false;
					continue;
				}

				materialTextures[m] = uploaded.GetValue();
				textureUploadCache.emplace(key, uploaded.GetValue());
				uploadedTextures.push_back(uploaded.GetValue());
				Log("ModernClientApp: material '%s' texture '%s' -> '%s' uploaded as image %llu "
				    "(%u x %u, %s)",
				    material.name.c_str(), material.textureFilename.c_str(), resolved.string().c_str(),
				    static_cast<unsigned long long>(uploaded.GetValue().Get()),
				    image.GetValue()->GetWidth(), image.GetValue()->GetHeight(),
				    ToString(image.GetValue()->GetFormat()));
			}

			Log("ModernClientApp: texture cache holds %zu decoded image(s); %zu uploaded to the GPU",
			    textureCache.GetCachedCount(), textureUploadCache.size());
		}

		// The per-section draw plan, logged once for evidence: every section's
		// index range, its material, and the texture handle it will bind (or the
		// fallback marker when there is none).
		for (size_t s = 0; s < sections.size(); ++s)
		{
			const MeshSection& section = sections[s];
			const bool   textured = section.materialIndex < materialTextures.size() &&
			                        materialTextures[section.materialIndex].IsValid();
			const char* binding   = "none (fallback color)";
			std::string texturedBinding;
			if (textured)
			{
				texturedBinding = std::to_string(materialTextures[section.materialIndex].Get()) +
				                  " (textured)";
				binding = texturedBinding.c_str();
			}
			const bool named = section.materialIndex < materials.size();
			Log("ModernClientApp: section %zu: indices [%u, %u) material %zu '%s' texture '%s' -> %s",
			    s, section.indexOffset, section.indexOffset + section.indexCount,
			    section.materialIndex,
			    named ? materials[section.materialIndex].name.c_str() : "?",
			    named ? materials[section.materialIndex].textureFilename.c_str() : "?",
			    binding);
		}
	}

	// 7. A camera that frames the model: the bounding box of the decoded
	//    geometry decides the distance; the transforms themselves are fixed
	//    for the life of the program.
	Vector3 minimum = mesh.GetVertices().front().position;
	Vector3 maximum = minimum;
	for (const MeshVertex& vertex : mesh.GetVertices())
	{
		minimum.x = std::min(minimum.x, vertex.position.x);
		minimum.y = std::min(minimum.y, vertex.position.y);
		minimum.z = std::min(minimum.z, vertex.position.z);
		maximum.x = std::max(maximum.x, vertex.position.x);
		maximum.y = std::max(maximum.y, vertex.position.y);
		maximum.z = std::max(maximum.z, vertex.position.z);
	}

	const Vector3 center = (minimum + maximum) * 0.5f;
	float radiusSq = 0.0f;
	for (const MeshVertex& vertex : mesh.GetVertices())
	{
		radiusSq = std::max(radiusSq, vertex.position.DistanceSq(center));
	}
	const float radius = std::sqrt(radiusSq);

	constexpr float kFovY = 60.0f * 3.14159265358979323846f / 180.0f;
	const float distance = radius * 2.5f + 0.05f;
	const float nearPlane = std::max(0.01f, distance - radius * 2.0f);
	const float farPlane = distance + radius * 3.0f;

	float world[16] = {};
	world[0] = world[5] = world[10] = world[15] = 1.0f;
	float view[16] = {};
	float projection[16] = {};

	// The character is viewed from the front (-Z side, looking toward +Z);
	// the distance frames the whole bounding sphere with margin.
	const Vector3 eye(center.x, center.y, center.z - distance);
	BuildLookAtRowMajor(eye, center, Vector3::Up, view);
	BuildPerspectiveRowMajor(kFovY, static_cast<float>(kClientWidth) / kClientHeight,
	                         nearPlane, farPlane, projection);

	Log("ModernClientApp: camera at (%.2f, %.2f, %.2f), bounding sphere radius %.2f",
	    eye.x, eye.y, eye.z, radius);

	// 8. The application loop: platform pump, input queue, one static draw
	//    per frame, real D3D11 present.
	Application application(std::move(platform));
	application.SetRenderer(&renderer);
	application.SetInputSystem(&inputSystem);
	application.SetInputSource(std::move(inputSource));

	application.SubscribeInput([&application](const InputEvent& event) {
		if (event.type == InputEventType::Key)
		{
			const KeyEvent& key = std::get<KeyEvent>(event.payload);
			if (key.code == KeyCode::Escape && key.pressed)
			{
				application.RequestStop();
			}
		}
	});

	// CLIENT-RENDER-002: one indexed draw per section, each binding its own
	// material's texture at its own index range. A section whose material
	// carries a real uploaded texture draws textured; one that does not draws
	// with the fallback color, which is only ever supplied to those sections.
	application.SetRenderCallback([&renderer, &characterMesh, &sections, &materialTextures,
	                               &world, &view, &projection](uint64_t frameIndex) {
		(void)frameIndex;

		renderer.Clear(RenderColor(0.07f, 0.09f, 0.14f, 1.0f));

		for (const MeshSection& section : sections)
		{
			const bool hasTexture = section.materialIndex < materialTextures.size() &&
			                        materialTextures[section.materialIndex].IsValid();

			MeshDrawCommand command;
			command.mesh             = characterMesh;
			command.texture          = hasTexture
			                               ? materialTextures[section.materialIndex]
			                               : ImageResourceHandle::MakeInvalid();
			command.worldMatrix      = world;
			command.viewMatrix       = view;
			command.projectionMatrix = projection;
			command.indexOffset      = section.indexOffset;
			command.indexCount       = section.indexCount;
			command.fallbackColor    = hasTexture ? RenderColor::White : kFallbackColor;

			renderer.DrawIndexedMesh(command);
		}
	});

	if (application.Initialize().IsError())
	{
		Log("ModernClientApp: application initialization failed");
		return 1;
	}

	const Status runStatus = application.Run();

	// 9. Explicit teardown, in real dependency order.
	//
	//    The Application now owns the platform, so the window is destroyed by
	//    its destructor at scope exit -- after every object declared above it,
	//    because destructors run in reverse declaration order. Releasing the
	//    GPU side here instead means the real order is:
	//
	//        textures -> mesh buffers -> device/swap chain/context -> window
	//
	//    which is the order D3D11 wants, since the swap chain is bound to the
	//    window handle. Doing it explicitly rather than by declaration order
	//    means the sequence does not change if a declaration is ever moved.
	for (const ImageResourceHandle texture : uploadedTextures)
	{
		uploader.ReleaseImage(texture);  // texture + SRV, device still alive
	}
	uploader.ReleaseMesh(characterMesh);  // vertex/index buffers, device still alive
	renderer.Shutdown();                 // swap chain, device, immediate context

	Log("ModernClientApp: stopped after %llu frames",
	    static_cast<unsigned long long>(application.GetFrameCount()));

	if (!namedTexturesAllLoaded)
	{
		Log("ModernClientApp: WARNING -- one or more named textures failed to load; the sections "
		    "that used them drew with the fallback color and are NOT a textured result");
	}
	// A clean window stop AND complete textured loading: a run whose named
	// textures did not all load is reported as a failure, so a fallback draw is
	// never mistaken for the textured character the milestone asked for.
	const bool success = runStatus.IsOk() && namedTexturesAllLoaded;
	return success ? 0 : 1;
}
