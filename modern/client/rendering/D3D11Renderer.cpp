// CLIENT-RENDER-001B: Direct3D 11 renderer implementation.
//
// Real GPU rendering: a hardware device, a window-bound swap chain, a
// depth-stencil buffer, an embedded HLSL pipeline compiled at Initialize
// time with D3DCompile (no build-time .cso step and no dependence on the
// current working directory), and an indexed triangle-list draw path fed
// by D3D11AssetUploader through opaque handles.
//
// Matrix convention: MeshDrawCommand carries row-major Mat4x4 data (see
// RenderingTypes.h) and the shaders below declare their constant buffer
// with #pragma pack_matrix(row_major), so a Mat4x4 uploads byte-for-byte
// and mul(matrix, vector) computes the row-major product the caller built.
// No transposition happens anywhere in this pipeline.

#include "rendering/D3D11Renderer.h"

#include "assets/MeshAsset.h"
#include "rendering/D3D11AssetUploader.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace Modern::Client
{
namespace
{

void Log(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	std::vfprintf(stderr, format, args);
	std::fputc('\n', stderr);
	va_end(args);
}

HRESULT LogAndReturn(HRESULT hr, const char* what)
{
	Log("D3D11Renderer: %s failed (HRESULT 0x%08X)", what, static_cast<unsigned>(hr));
	return hr;
}

template <typename T>
void SafeRelease(T*& ptr) noexcept
{
	if (ptr != nullptr)
	{
		ptr->Release();
		ptr = nullptr;
	}
}

// The per-frame constant buffer: three row-major matrices, the fallback
// color used when no texture is bound, and the flag choosing between the
// two pixel-shader paths. Size is a multiple of 16 (224 = 14 * 16), which
// D3D11 requires of constant buffers.
struct MeshConstants
{
	float world[16];
	float view[16];
	float projection[16];
	float fallbackColor[4];
	bool  useTexture;
	float padding[3];
};

static_assert(sizeof(MeshConstants) == 224, "constant buffer must be 224 bytes (14 * 16)");
static_assert(offsetof(MeshConstants, fallbackColor) == 192, "fallbackColor must follow the three matrices");
static_assert(offsetof(MeshConstants, useTexture) == 208, "useTexture must follow fallbackColor");

// The vertex declaration must agree with MeshVertex byte for byte: the
// asset layer static_asserts MeshVertex == kMeshVertexBytes; this is the
// same promise from the GPU side.
static_assert(sizeof(MeshVertex) == kMeshVertexBytes, "MeshVertex must match the serialised vertex size");
static_assert(offsetof(MeshVertex, position) == 0, "position must lead the vertex");
static_assert(offsetof(MeshVertex, normal) == 12, "normal must follow position");
static_assert(offsetof(MeshVertex, u) == 24, "uv must follow normal");

// ---------------------------------------------------------------------------
// Embedded HLSL
//
// Compiled at Initialize with D3DCompile: the shader source lives in this
// translation unit, so the executable needs no external .cso file and no
// assumption about the working directory it was launched from. A compile
// failure prints the compiler's own diagnostic text and Initialization
// fails loudly -- a renderer that cannot build its shaders never claims
// success.
// ---------------------------------------------------------------------------

constexpr const char* kVertexShaderSource = R"(
#pragma pack_matrix(row_major)

cbuffer MeshConstants : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
    float4 fallbackColor;
    bool useTexture;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct VSInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 worldPosition = mul(float4(input.position, 1.0f), world);
    float4 viewPosition = mul(worldPosition, view);
    output.position = mul(viewPosition, projection);
    output.uv = input.uv;
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    if (useTexture)
    {
        return g_texture.Sample(g_sampler, input.uv);
    }
    return fallbackColor;
}
)";

void LogShaderErrors(const char* stage, ID3DBlob* errors) noexcept
{
	if (errors != nullptr)
	{
		const char* message = reinterpret_cast<const char*>(errors->GetBufferPointer());
		Log("D3D11Renderer: %s shader compilation failed:\n%s", stage,
			message ? message : "(no diagnostic text)");
	}
	else
	{
		Log("D3D11Renderer: %s shader compilation failed (no error blob)", stage);
	}
}

} // namespace

D3D11Renderer::~D3D11Renderer()
{
	Shutdown();
}

Status D3D11Renderer::Initialize(const RendererConfig& config)
{
	if (m_state == RendererState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != RendererState::Uninitialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	if (!config.IsValid())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// Fullscreen display modes are not implemented by this backend yet;
	// refusing them beats silently running windowed.
	if (config.displayMode != DisplayMode::Windowed)
	{
		Log("D3D11Renderer: display mode %s is not supported (Windowed only)",
			ToString(config.displayMode));
		return Status(ErrorCode::NotAllowed);
	}

	if (config.windowHandle == nullptr)
	{
		Log("D3D11Renderer: no window handle in RendererConfig; cannot bind a swap chain");
		return Status(ErrorCode::InvalidArgument);
	}

	m_config = config;

	if (CreateDeviceAndSwapChain(m_config.windowHandle).IsError())
	{
		ReleaseResources();
		return Status(ErrorCode::InvalidArgument);
	}

	if (CreateRenderTargetView().IsError() ||
	    CreateDepthStencilView().IsError() ||
	    CreateShaders().IsError() ||
	    CreateConstantBuffer().IsError() ||
	    CreateSamplerState().IsError() ||
	    CreateDefaultTexture().IsError() ||
	    SetupViewport().IsError())
	{
		ReleaseResources();
		return Status(ErrorCode::InvalidArgument);
	}

	m_state = RendererState::Initialized;
	Log("D3D11Renderer: initialized %u x %u (windowed, vsync %s)",
		m_config.width, m_config.height, m_config.vsync ? "on" : "off");
	return Ok();
}

Status D3D11Renderer::BeginFrame()
{
	if (m_state == RendererState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != RendererState::Initialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	if (m_renderTargetView == nullptr || m_depthStencilView == nullptr)
	{
		Log("D3D11Renderer: BeginFrame without render targets (a previous resize failed)");
		return Status(ErrorCode::InvalidState);
	}

	// Bind the frame's targets and viewport every BeginFrame: nothing in
	// the draw path unbinds them, and a resize recreates them.
	m_context->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	m_context->RSSetViewports(1, &m_viewport);

	m_inFrame = true;
	m_state = RendererState::InFrame;
	return Ok();
}

Status D3D11Renderer::EndFrame()
{
	if (m_state == RendererState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != RendererState::InFrame)
	{
		return Status(ErrorCode::InvalidState);
	}

	m_inFrame = false;
	m_state = RendererState::Initialized;

	// The real presentation call: this is what puts the cleared, drawn
	// back buffer on screen.
	const HRESULT hr = m_swapChain->Present(m_config.vsync ? 1 : 0, 0);
	if (FAILED(hr))
	{
		LogAndReturn(hr, "Present");
		return Status(ErrorCode::InvalidArgument);
	}

	return Ok();
}

Status D3D11Renderer::Clear(const RenderColor& color)
{
	if (m_state == RendererState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != RendererState::InFrame)
	{
		return Status(ErrorCode::InvalidState);
	}

	const float clearColor[4] = { color.r, color.g, color.b, color.a };
	m_context->ClearRenderTargetView(m_renderTargetView, clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	return Ok();
}

Status D3D11Renderer::Resize(uint32_t width, uint32_t height)
{
	if (m_state == RendererState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != RendererState::Initialized)
	{
		// Cannot resize while InFrame or Uninitialized.
		return Status(ErrorCode::InvalidState);
	}

	if (width == 0 || height == 0)
	{
		// A minimized window posts a 0x0 WM_SIZE; the Application loop
		// already filters those, and this is the backstop: refuse rather
		// than ResizeBuffers(0, 0).
		return Status(ErrorCode::InvalidArgument);
	}

	if (m_swapChain == nullptr)
	{
		Log("D3D11Renderer: Resize without a swap chain");
		return Status(ErrorCode::InvalidState);
	}

	// Every outstanding reference to the back buffer must be released
	// before ResizeBuffers: the render-target view, the depth-stencil view
	// and the depth-stencil buffer it was created from. Unbinding the OM
	// stage first is what makes the release unambiguous.
	m_context->OMSetRenderTargets(0, nullptr, nullptr);
	SafeRelease(m_renderTargetView);
	SafeRelease(m_depthStencilView);
	SafeRelease(m_depthStencilBuffer);

	const HRESULT hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr))
	{
		LogAndReturn(hr, "ResizeBuffers");
		return Status(ErrorCode::InvalidArgument);
	}

	// Recreate everything that depended on the back buffer size.
	if (CreateRenderTargetView().IsError() ||
	    CreateDepthStencilView().IsError() ||
	    SetupViewport().IsError())
	{
		ReleaseResources();
		return Status(ErrorCode::InvalidArgument);
	}

	m_config.width = width;
	m_config.height = height;

	Log("D3D11Renderer: resized to %u x %u", width, height);
	return Ok();
}

Status D3D11Renderer::DrawIndexedMesh(const MeshDrawCommand& command)
{
	if (m_state == RendererState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != RendererState::InFrame)
	{
		return Status(ErrorCode::InvalidState);
	}

	if (!command.mesh.IsValid())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	if (command.worldMatrix == nullptr || command.viewMatrix == nullptr ||
	    command.projectionMatrix == nullptr)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	if (m_assetUploader == nullptr)
	{
		Log("D3D11Renderer: DrawIndexedMesh without an asset uploader wired");
		return Status(ErrorCode::InvalidState);
	}

	// Resolve the opaque handles to live GPU objects. The mesh handle is
	// required; the texture is optional and falls back to the default
	// texture plus the command's fallback color.
	D3D11MeshGpuResources meshGpu;
	if (!m_assetUploader->TryResolveMesh(command.mesh, meshGpu))
	{
		Log("D3D11Renderer: mesh handle %llu does not name a live GPU mesh",
			static_cast<unsigned long long>(command.mesh.Get()));
		return Status(ErrorCode::NotFound);
	}

	// CLIENT-RENDER-002: an optional index range draws one section of the
	// mesh. The count is the only optional half -- 0 means the whole mesh,
	// which is every draw that existed before sections did. A non-zero range
	// must be a whole number of triangles and must lie inside the uploaded
	// index buffer; both are checked here rather than in the driver, so a bad
	// range is a refusal and not a draw of memory past the buffer.
	uint32_t drawCount  = meshGpu.indexCount;
	uint32_t drawOffset = 0;
	if (command.indexCount != 0)
	{
		if ((command.indexCount % 3u) != 0u ||
		    command.indexOffset > meshGpu.indexCount ||
		    command.indexCount > meshGpu.indexCount - command.indexOffset)
		{
			Log("D3D11Renderer: index range [%u, %u) is not a triangle run inside a %u-index mesh",
				command.indexOffset, command.indexOffset + command.indexCount, meshGpu.indexCount);
			return Status(ErrorCode::InvalidArgument);
		}
		drawCount  = command.indexCount;
		drawOffset = command.indexOffset;
	}

	D3D11ImageGpuResources imageGpu;
	const bool hasTexture =
	    command.texture.IsValid() && m_assetUploader->TryResolveImage(command.texture, imageGpu);
	if (command.texture.IsValid() && !hasTexture)
	{
		Log("D3D11Renderer: texture handle %llu is not live; using fallback color",
			static_cast<unsigned long long>(command.texture.Get()));
	}

	// Upload the per-frame constants.
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	if (SUCCEEDED(m_context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		MeshConstants* constants = reinterpret_cast<MeshConstants*>(mapped.pData);
		std::memcpy(constants->world, command.worldMatrix, sizeof(constants->world));
		std::memcpy(constants->view, command.viewMatrix, sizeof(constants->view));
		std::memcpy(constants->projection, command.projectionMatrix, sizeof(constants->projection));
		constants->fallbackColor[0] = command.fallbackColor.r;
		constants->fallbackColor[1] = command.fallbackColor.g;
		constants->fallbackColor[2] = command.fallbackColor.b;
		constants->fallbackColor[3] = command.fallbackColor.a;
		constants->useTexture = hasTexture;
		m_context->Unmap(m_constantBuffer, 0);
	}
	else
	{
		Log("D3D11Renderer: constant buffer Map failed; drawing with stale constants");
	}

	const UINT vertexStride = sizeof(MeshVertex);
	const UINT vertexOffset = 0;
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetVertexBuffers(0, 1, &meshGpu.vertexBuffer, &vertexStride, &vertexOffset);
	m_context->IASetIndexBuffer(meshGpu.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_context->VSSetShader(m_vertexShader, nullptr, 0);
	m_context->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	// The same cbuffer holds the pixel shader's fallbackColor and useTexture, so
	// it is bound to the PS as well. Leaving it bound to the VS only reads as
	// zero in the pixel stage -- every fragment would take the fallback branch
	// with a zeroed (black) colour and no texture, whatever the caller asked
	// for, which is exactly what a textured mesh drawn black looks like.
	m_context->PSSetConstantBuffers(0, 1, &m_constantBuffer);
	m_context->PSSetShader(m_pixelShader, nullptr, 0);

	ID3D11ShaderResourceView* shaderResourceView =
	    hasTexture ? imageGpu.shaderResourceView : m_defaultTextureSrv;
	m_context->PSSetShaderResources(0, 1, &shaderResourceView);
	m_context->PSSetSamplers(0, 1, &m_samplerState);

	m_context->RSSetViewports(1, &m_viewport);
	m_context->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	m_context->DrawIndexed(drawCount, drawOffset, 0);
	return Ok();
}

void D3D11Renderer::Shutdown() noexcept
{
	ReleaseResources();
	m_state = RendererState::ShutdownState;
	m_inFrame = false;
}

bool D3D11Renderer::IsInitialized() const noexcept
{
	return m_state == RendererState::Initialized || m_state == RendererState::InFrame;
}

Status D3D11Renderer::CreateDeviceAndSwapChain(void* windowHandle)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount        = 2;
	swapChainDesc.BufferDesc.Width    = m_config.width;
	swapChainDesc.BufferDesc.Height   = m_config.height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format   = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage         = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow        = static_cast<HWND>(windowHandle);
	swapChainDesc.SampleDesc.Count     = 1;
	swapChainDesc.SampleDesc.Quality   = 0;
	swapChainDesc.Windowed            = TRUE;
	swapChainDesc.SwapEffect          = DXGI_SWAP_EFFECT_DISCARD;

	const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT createDeviceFlags = 0;
#if defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL obtainedLevel = D3D_FEATURE_LEVEL_10_0;
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		featureLevels,
		static_cast<UINT>(sizeof(featureLevels) / sizeof(featureLevels[0])),
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_swapChain,
		&m_device,
		&obtainedLevel,
		&m_context);

	// The debug layer is a development extra, not a requirement: without
	// it installed the flagged call fails, so retry without the flag.
	if (FAILED(hr) && (createDeviceFlags & D3D11_CREATE_DEVICE_DEBUG) != 0)
	{
		Log("D3D11Renderer: device creation with the debug layer failed (HRESULT 0x%08X); retrying without it",
			static_cast<unsigned>(hr));
		createDeviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createDeviceFlags,
			featureLevels,
			static_cast<UINT>(sizeof(featureLevels) / sizeof(featureLevels[0])),
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&m_swapChain,
			&m_device,
			&obtainedLevel,
			&m_context);
	}

	if (FAILED(hr))
	{
		LogAndReturn(hr, "D3D11CreateDeviceAndSwapChain");
		return Status(ErrorCode::InvalidArgument);
	}

	Log("D3D11Renderer: device created (feature level %u.%u)",
		obtainedLevel >> 8, obtainedLevel & 0xFF);
	return Ok();
}

Status D3D11Renderer::CreateRenderTargetView()
{
	ID3D11Texture2D* backBuffer = nullptr;
	const HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	if (FAILED(hr))
	{
		LogAndReturn(hr, "IDXGISwapChain::GetBuffer");
		return Status(ErrorCode::InvalidArgument);
	}

	const HRESULT viewHr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);
	SafeRelease(backBuffer);
	if (FAILED(viewHr))
	{
		LogAndReturn(viewHr, "CreateRenderTargetView");
		return Status(ErrorCode::InvalidArgument);
	}

	return Ok();
}

Status D3D11Renderer::CreateDepthStencilView()
{
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width            = m_config.width;
	depthDesc.Height           = m_config.height;
	depthDesc.MipLevels        = 1;
	depthDesc.ArraySize        = 1;
	depthDesc.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage            = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags        = D3D11_BIND_DEPTH_STENCIL;

	const HRESULT bufferHr = m_device->CreateTexture2D(&depthDesc, nullptr, &m_depthStencilBuffer);
	if (FAILED(bufferHr))
	{
		LogAndReturn(bufferHr, "CreateTexture2D (depth stencil)");
		return Status(ErrorCode::InvalidArgument);
	}

	const HRESULT viewHr = m_device->CreateDepthStencilView(m_depthStencilBuffer, nullptr, &m_depthStencilView);
	if (FAILED(viewHr))
	{
		LogAndReturn(viewHr, "CreateDepthStencilView");
		return Status(ErrorCode::InvalidArgument);
	}

	m_context->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	return Ok();
}

Status D3D11Renderer::CreateShaders()
{
	ID3DBlob* vertexBlob = nullptr;
	ID3DBlob* pixelBlob  = nullptr;
	ID3DBlob* errors     = nullptr;

	HRESULT hr = D3DCompile(
		kVertexShaderSource,
		std::strlen(kVertexShaderSource),
		"ModernClientMesh.vs",
		nullptr,
		nullptr,
		"VSMain",
		"vs_4_0",
		0,
		0,
		&vertexBlob,
		&errors);
	if (FAILED(hr))
	{
		LogShaderErrors("vertex", errors);
		SafeRelease(vertexBlob);
		SafeRelease(errors);
		return Status(ErrorCode::InvalidArgument);
	}
	SafeRelease(errors);

	hr = D3DCompile(
		kVertexShaderSource,
		std::strlen(kVertexShaderSource),
		"ModernClientMesh.ps",
		nullptr,
		nullptr,
		"PSMain",
		"ps_4_0",
		0,
		0,
		&pixelBlob,
		&errors);
	if (FAILED(hr))
	{
		LogShaderErrors("pixel", errors);
		SafeRelease(vertexBlob);
		SafeRelease(pixelBlob);
		SafeRelease(errors);
		return Status(ErrorCode::InvalidArgument);
	}
	SafeRelease(errors);

	hr = m_device->CreateVertexShader(
		vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(hr))
	{
		LogAndReturn(hr, "CreateVertexShader");
		SafeRelease(vertexBlob);
		SafeRelease(pixelBlob);
		return Status(ErrorCode::InvalidArgument);
	}

	hr = m_device->CreatePixelShader(
		pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(hr))
	{
		LogAndReturn(hr, "CreatePixelShader");
		SafeRelease(vertexBlob);
		SafeRelease(pixelBlob);
		return Status(ErrorCode::InvalidArgument);
	}

	const D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = m_device->CreateInputLayout(
		inputLayout,
		static_cast<UINT>(sizeof(inputLayout) / sizeof(inputLayout[0])),
		vertexBlob->GetBufferPointer(),
		vertexBlob->GetBufferSize(),
		&m_inputLayout);

	SafeRelease(vertexBlob);
	SafeRelease(pixelBlob);

	if (FAILED(hr))
	{
		LogAndReturn(hr, "CreateInputLayout");
		return Status(ErrorCode::InvalidArgument);
	}

	Log("D3D11Renderer: shaders compiled and input layout created");
	return Ok();
}

Status D3D11Renderer::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth           = sizeof(MeshConstants);
	desc.Usage               = D3D11_USAGE_DYNAMIC;
	desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;

	const HRESULT hr = m_device->CreateBuffer(&desc, nullptr, &m_constantBuffer);
	if (FAILED(hr))
	{
		LogAndReturn(hr, "CreateBuffer (constants)");
		return Status(ErrorCode::InvalidArgument);
	}

	return Ok();
}

Status D3D11Renderer::CreateSamplerState()
{
	D3D11_SAMPLER_DESC desc = {};
	desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU      = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV      = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW      = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	// Every texture this renderer samples is a single 2D level: the uploader
	// creates MipLevels=1 from ImageAsset (CLIENT-009 stores one 2D image). A
	// MIN_*_MIP_* filter still computes a level-of-detail from the UV
	// derivatives, and for a texture minified on screen that LOD can exceed
	// mip 0. Sampling a mip level that does not exist yields black on most
	// drivers, so MaxLOD is clamped to 0: only mip 0 is ever addressed, and the
	// (existing) MIN_MAG linear filtering still smooths within that level. A
	// future milestone that uploads a real mip chain can lift this clamp.
	desc.MinLOD        = 0.0f;
	desc.MaxLOD        = 0.0f;

	const HRESULT hr = m_device->CreateSamplerState(&desc, &m_samplerState);
	if (FAILED(hr))
	{
		LogAndReturn(hr, "CreateSamplerState");
		return Status(ErrorCode::InvalidArgument);
	}

	return Ok();
}

Status D3D11Renderer::CreateDefaultTexture()
{
	// The 1x1 white texture bound whenever a draw has no real texture: the
	// pixel shader's fallback path ignores the sample and returns
	// fallbackColor, so an untextured mesh draws in a visible flat color
	// while the pipeline stays fully bound.
	const uint8_t white[4] = { 255, 255, 255, 255 };

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width            = 1;
	desc.Height           = 1;
	desc.MipLevels        = 1;
	desc.ArraySize        = 1;
	desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage            = D3D11_USAGE_DEFAULT;
	desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem     = white;
	initData.SysMemPitch = sizeof(white);

	ID3D11Texture2D* texture = nullptr;
	HRESULT hr = m_device->CreateTexture2D(&desc, &initData, &texture);
	if (FAILED(hr))
	{
		LogAndReturn(hr, "CreateTexture2D (default white)");
		return Status(ErrorCode::InvalidArgument);
	}

	hr = m_device->CreateShaderResourceView(texture, nullptr, &m_defaultTextureSrv);
	SafeRelease(texture);
	if (FAILED(hr))
	{
		LogAndReturn(hr, "CreateShaderResourceView (default white)");
		return Status(ErrorCode::InvalidArgument);
	}

	return Ok();
}

Status D3D11Renderer::SetupViewport()
{
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Width    = static_cast<float>(m_config.width);
	m_viewport.Height   = static_cast<float>(m_config.height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	return Ok();
}

void D3D11Renderer::ReleaseShaders()
{
	SafeRelease(m_defaultTextureSrv);
	SafeRelease(m_samplerState);
	SafeRelease(m_constantBuffer);
	SafeRelease(m_inputLayout);
	SafeRelease(m_pixelShader);
	SafeRelease(m_vertexShader);
}

void D3D11Renderer::ReleaseResources()
{
	// Order: pipeline state first (it references the buffers), then the
	// target views and the swap chain (which reference the back buffer),
	// then the immediate context, then the device. Every pointer is
	// nulled, so a second Shutdown is a no-op.
	ReleaseShaders();
	SafeRelease(m_depthStencilView);
	SafeRelease(m_depthStencilBuffer);
	SafeRelease(m_renderTargetView);
	SafeRelease(m_swapChain);
	SafeRelease(m_context);
	SafeRelease(m_device);
}

} // namespace Modern::Client
