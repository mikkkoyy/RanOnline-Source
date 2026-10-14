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
// The character is drawn every frame from the real decoded mesh: a
// triangle-list indexed draw of the uploaded vertex/index buffers, with a
// visible flat fallback color (this ship's skin textures are MTF, and MTF
// texture upload is not part of this milestone -- the fallback path is the
// honest visible result).
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
#include "assets/MeshAsset.h"
#include "assets/MxfMeshTransform.h"
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
#include <memory>
#include <string>

using Modern::Vector3;
using Modern::Client::Application;
using Modern::Client::D3D11AssetUploader;
using Modern::Client::D3D11Renderer;
using Modern::Client::DisplayMode;
using Modern::Client::FileSystemResourceProvider;
using Modern::Client::IInputSource;
using Modern::Client::ImageResourceHandle;
using Modern::Client::InputEvent;
using Modern::Client::InputEventType;
using Modern::Client::InputSystem;
using Modern::Client::KeyEvent;
using Modern::Client::KeyCode;
using Modern::Client::MeshAsset;
using Modern::Client::MeshDrawCommand;
using Modern::Client::MeshResourceHandle;
using Modern::Client::MeshVertex;
using Modern::Client::MxfMeshTransform;
using Modern::Client::RenderColor;
using Modern::Client::RendererConfig;
using Modern::Client::ResourceManager;
using Modern::Client::ResourceId;
using Modern::Client::ResourceData;
using Modern::Status;
using Modern::Client::Win32InputSource;
using Modern::Client::Win32PlatformEvents;
using Modern::Client::XMeshDecoder;

namespace
{

constexpr uint32_t kClientWidth  = 960;
constexpr uint32_t kClientHeight = 540;

constexpr const char* kSkinMeshPath = "data/skin/m_luffy_timeskip.mxf";

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

	const MeshResourceHandle characterMesh = meshHandle.GetValue();
	application.SetRenderCallback([&renderer, &characterMesh, &world, &view, &projection](
		uint64_t frameIndex) {
		(void)frameIndex;

		renderer.Clear(RenderColor(0.07f, 0.09f, 0.14f, 1.0f));

		MeshDrawCommand command;
		command.mesh             = characterMesh;
		command.texture          = ImageResourceHandle::MakeInvalid();
		command.worldMatrix      = world;
		command.viewMatrix       = view;
		command.projectionMatrix = projection;
		// No texture is bound for this milestone: the shader's fallback path
		// makes this flat color the visible result.
		command.fallbackColor = RenderColor(0.62f, 0.76f, 0.92f, 1.0f);

		renderer.DrawIndexedMesh(command);
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
	//        buffers -> device/swap chain/context -> window
	//
	//    which is the order D3D11 wants, since the swap chain is bound to the
	//    window handle. Doing it explicitly rather than by declaration order
	//    means the sequence does not change if a declaration is ever moved.
	uploader.ReleaseMesh(characterMesh);  // vertex/index buffers, device still alive
	renderer.Shutdown();                 // swap chain, device, immediate context

	Log("ModernClientApp: stopped after %llu frames",
		static_cast<unsigned long long>(application.GetFrameCount()));
	return runStatus.IsOk() ? 0 : 1;
}
