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
#include "assets/TextureLoader.h"
#include "assets/XMeshDecoder.h"
#include "input/InputEvents.h"
#include "input/InputSystem.h"
#include "input/platform/Win32InputSource.h"
#include "input/platform/Win32PlatformEvents.h"
#include "math/Vector3.h"
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
using Modern::Status;
using Modern::Client::SkinPiece;
using Modern::Client::SkinPieceContainer;
using Modern::Client::SkinPieceType;
using Modern::Client::TextureCache;
using Modern::Client::TextureLoader;
using Modern::Client::Win32InputSource;
using Modern::Client::Win32PlatformEvents;
using Modern::Client::XMeshDecoder;

namespace
{

constexpr uint32_t kClientWidth  = 960;
constexpr uint32_t kClientHeight = 540;

// The CLIENT-RENDER-002 regression mesh. Still the default so that a plain
// run of the client keeps proving the MXF path works.
constexpr const char* kSkinMeshPath = "data/skin/m_luffy_timeskip.mxf";

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
		"  ModernClientApp.exe --audit-textures [--asset-root <path>] [--report <json>]\n"
		"                       [--decode-limit <n-per-format>]\n"
		"\n"
		"  --asset-root <path>            RAN client tree to load data from\n"
		"                                 (default: $RAN_ASSET_ROOT, then the current directory)\n"
		"  --mesh <path>                  Mesh to render, relative to the asset root\n"
		"                                 (default: data/skin/m_luffy_timeskip.mxf)\n"
		"  --skin-piece <path>            RAN wearing file under data/skinobject to take the\n"
		"                                 mesh from, e.g. data/skinobject/CUN0001_m.cps.\n"
		"                                 Overrides --mesh when both are given.\n"
		"  --audit-textures               Headless: inventory the texture tree, report per-format\n"
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

// Row-major left-handed look-at view matrix (the layout
// D3D11Renderer's `#pragma pack_matrix(row_major)` shader expects).
void BuildLookAtRowMajor(const Vector3& eye, const Vector3& target, const Vector3& up, float* out)
{
	const Vector3 zaxis = Normalize(target - eye);
	const Vector3 xaxis = Normalize(Cross(up, zaxis));
	const Vector3 yaxis = Cross(zaxis, xaxis);

	out[0]  = xaxis.x; out[1]  = yaxis.x; out[2]  = zaxis.x; out[3]  = 0.0f;
	out[4]  = xaxis.y; out[5]  = yaxis.y; out[6]  = zaxis.y; out[7]  = 0.0f;
	out[8]  = xaxis.z; out[9]  = yaxis.z; out[10] = zaxis.z; out[11] = 0.0f;
	out[12] = -Dot(xaxis, eye);
	out[13] = -Dot(yaxis, eye);
	out[14] = -Dot(zaxis, eye);
	out[15] = 1.0f;
}

// Row-major left-handed perspective projection matrix.
void BuildPerspectiveRowMajor(float fovYRadians, float aspect, float nearPlane, float farPlane, float* out)
{
	const float cotHalfFov = 1.0f / std::tan(fovYRadians * 0.5f);
	const float w = cotHalfFov / aspect;
	const float h = cotHalfFov;
	const float q = farPlane / (farPlane - nearPlane);

	out[0]  = w; out[1]  = 0.0f; out[2]  = 0.0f; out[3]  = 0.0f;
	out[4]  = 0.0f; out[5]  = h; out[6]  = 0.0f; out[7]  = 0.0f;
	out[8]  = 0.0f; out[9]  = 0.0f; out[10] = q; out[11] = 1.0f;
	out[12] = 0.0f; out[13] = 0.0f;
	out[14] = -nearPlane * farPlane / (farPlane - nearPlane);
	out[15] = 0.0f;
}

} // namespace

int main(int argc, char** argv)
{
	std::string assetRoot;
	std::string meshPath;
	std::string skinPiecePath;
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
	const Modern::Result<MeshResourceHandle> meshHandle = uploader.UploadMesh(mesh);
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