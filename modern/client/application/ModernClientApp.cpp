// CLIENT-RENDER-001B: the modern client production executable.
//
// ModernClientApp wires the real client stack into one windowed program:
//
//   Win32PlatformEvents  -> the native 960x540 window and its message pump
//   Win32InputSource     -> Win32 input translated to platform-independent events
//   D3D11Renderer        -> the Direct3D 11 backend (real frames, real Present)
//   D3D11AssetUploader   -> CPU assets uploaded to real GPU buffers/textures
//   asset pipeline       -> m_luffy_timeskip.mxf -> MxfMeshTransform
//                           -> XMeshDecoder -> MeshAsset -> GPU upload
//   Application          -> the frame loop, with a static character draw
//
// The character is drawn every frame from the real decoded mesh, one indexed
// draw per MeshSection, each binding its own material's PNG texture
// (blackcatyb_luffy_timeskip.png on the body, blackcatyb_luffy_timeskip_eye.png
// on the eye) at its own index range. The textures are resolved by name through
// TextureLoader over the ASURA texture tree and uploaded once each; a material
// that names no texture, or whose named texture cannot be resolved or decoded,
// falls back to a clearly-labelled diagnostic color and is reported as such --
// it is never counted as a textured draw.
//
// Usage:
//   ModernClientApp.exe [--asset-root <path>]
//
// --asset-root names the RAN client tree to load data/skin/m_luffy_timeskip.mxf
// from. Without it, the RAN_ASSET_ROOT environment variable is tried, then the
// current directory. Escape or closing the window exits; the process always
// returns 0 on a clean stop and non-zero on any initialization failure.

// Windows.h (pulled in by the D3D11 backend headers below) defines
// min/max as macros and renames GetMessage to GetMessageA/W. The first
// would break std::min/std::max below; the second would rename every
// Modern::Result::GetMessage() call in this file. NOMINMAX and the
// #undef after the includes keep both languages speaking.
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
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using Modern::Vector3;
using Modern::Client::Application;
using Modern::Client::D3D11AssetUploader;
using Modern::Client::D3D11Renderer;
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
using Modern::Client::RenderColor;
using Modern::Client::RendererConfig;
using Modern::Client::ResourceManager;
using Modern::Client::ResourceId;
using Modern::Client::ResourceData;
using Modern::Status;
using Modern::Client::TextureLoader;
using Modern::Client::Win32InputSource;
using Modern::Client::Win32PlatformEvents;
using Modern::Client::XMeshDecoder;

namespace
{

constexpr uint32_t kClientWidth  = 960;
constexpr uint32_t kClientHeight = 540;

constexpr const char* kSkinMeshPath  = "data/skin/m_luffy_timeskip.mxf";
constexpr const char* kTexturesDir   = "textures";

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
		"  ModernClientApp.exe [--asset-root <path>]\n"
		"\n"
		"  --asset-root <path>  RAN client tree to load the character from\n"
		"                       (default: $RAN_ASSET_ROOT, then the current directory)\n"
		"\n"
		"  The window is 960x540. Escape or closing the window exits.\n");
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
	//    file -> ResourceManager -> MXF transform -> X decoder -> MeshAsset.
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

	const Modern::Result<ResourceId> skinMeshId = ResourceId::Create(kSkinMeshPath);
	if (skinMeshId.IsError())
	{
		Log("ModernClientApp: bad resource id '%s' (%s)", kSkinMeshPath, skinMeshId.GetMessage());
		return 1;
	}

	const Modern::Result<ResourceData> skinFileData = resourceManager.Load(skinMeshId.GetValue());
	if (skinFileData.IsError())
	{
		Log("ModernClientApp: failed to load '%s' (%s) -- is the asset root a RAN client tree?",
			kSkinMeshPath, skinFileData.GetMessage());
		return 1;
	}

	MxfMeshTransform mxfTransform;
	const Modern::Result<ResourceData> xFileData = mxfTransform.Transform(skinFileData.GetValue());
	if (xFileData.IsError())
	{
		Log("ModernClientApp: MXF transform of '%s' failed (%s)",
			kSkinMeshPath, xFileData.GetMessage());
		return 1;
	}

	XMeshDecoder xMeshDecoder;
	const Modern::Result<MeshAsset> skinMesh = xMeshDecoder.DecodeMesh(xFileData.GetValue());
	if (skinMesh.IsError())
	{
		Log("ModernClientApp: X mesh decode of '%s' failed (%s)",
			kSkinMeshPath, skinMesh.GetMessage());
		return 1;
	}

	const MeshAsset& mesh = skinMesh.GetValue();
	Log("ModernClientApp: decoded %s: %zu vertices, %zu indices, %zu triangles",
		kSkinMeshPath, mesh.GetVertexCount(), mesh.GetIndexCount(), mesh.GetTriangleCount());

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
	//     content magic -- PNG here, with DDS/MTF handled by the same dispatch.
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
			for (size_t m = 0; m < materials.size(); ++m)
			{
				const MeshMaterial& material = materials[m];
				if (material.textureFilename.empty())
				{
					// A material with no texture is a real, legal shape; it draws
					// with the fallback color by design, which is not a failure.
					continue;
				}

				// Share the handle with an earlier material that named the same
				// file: one upload per unique image, not one per material.
				ImageResourceHandle reused;
				bool alreadyUploaded = false;
				for (size_t k = 0; k < m; ++k)
				{
					if (materials[k].textureFilename == material.textureFilename &&
					    materialTextures[k].IsValid())
					{
						reused = materialTextures[k];
						alreadyUploaded = true;
						break;
					}
				}
				if (alreadyUploaded)
				{
					materialTextures[m] = reused;
					continue;
				}

				const std::filesystem::path resolved = textureLoader.Find(material.textureFilename);
				if (resolved.empty())
				{
					Log("ModernClientApp: material '%s' names texture '%s' that is not in the "
					    "texture tree '%s'",
					    material.name.c_str(), material.textureFilename.c_str(),
					    textureRoot.string().c_str());
					namedTexturesAllLoaded = false;
					continue;
				}

				const Modern::Result<ImageAsset> image = textureLoader.Load(material.textureFilename);
				if (image.IsError())
				{
					Log("ModernClientApp: material '%s' texture '%s' (%s) failed to decode (%s)",
					    material.name.c_str(), material.textureFilename.c_str(),
					    resolved.string().c_str(), image.GetMessage());
					namedTexturesAllLoaded = false;
					continue;
				}

				const Modern::Result<ImageResourceHandle> uploaded =
				    uploader.UploadImage(image.GetValue());
				if (uploaded.IsError())
				{
					Log("ModernClientApp: material '%s' texture '%s' (%s) failed to upload (%s)",
					    material.name.c_str(), material.textureFilename.c_str(),
					    resolved.string().c_str(), uploaded.GetMessage());
					namedTexturesAllLoaded = false;
					continue;
				}

				materialTextures[m] = uploaded.GetValue();
				uploadedTextures.push_back(uploaded.GetValue());
				Log("ModernClientApp: material '%s' texture '%s' -> '%s' uploaded as image %llu "
				    "(%u x %u, %s)",
				    material.name.c_str(), material.textureFilename.c_str(), resolved.string().c_str(),
				    static_cast<unsigned long long>(uploaded.GetValue().Get()),
				    image.GetValue().GetWidth(), image.GetValue().GetHeight(),
				    ToString(image.GetValue().GetFormat()));
			}
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
