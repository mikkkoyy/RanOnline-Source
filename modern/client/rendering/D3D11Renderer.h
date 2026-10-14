#pragma once

// CLIENT-RENDER-001: Direct3D 11 renderer implementation.
//
// Concrete implementation of IRenderer using Direct3D 11 and DXGI.
// Provides real GPU rendering with swap chain presentation.
//
// This header is the D3D11 backend's own: it exposes the D3D11 object
// pointers the asset uploader binds against and holds a D3D11_VIEWPORT
// member, so it includes <d3d11.h>. The platform-independent boundary
// (Renderer.h, RenderingTypes.h, AssetUpload.h) is where D3D11 types
// never appear.

#include "Renderer.h"
#include "AssetUpload.h"

#include <Windows.h>
#include <d3d11.h>

#include <cstdint>
#include <memory>

namespace Modern::Client
{

class D3D11AssetUploader;

class D3D11Renderer final : public IRenderer
{
public:
    D3D11Renderer() = default;
    ~D3D11Renderer() override;

    D3D11Renderer(const D3D11Renderer&) = delete;
    D3D11Renderer& operator=(const D3D11Renderer&) = delete;
    D3D11Renderer(D3D11Renderer&&) = delete;
    D3D11Renderer& operator=(D3D11Renderer&&) = delete;

    // IRenderer implementation
    Status Initialize(const RendererConfig& config) override;
    Status BeginFrame() override;
    Status EndFrame() override;
    Status Clear(const RenderColor& color) override;
    Status Resize(uint32_t width, uint32_t height) override;
    Status DrawIndexedMesh(const MeshDrawCommand& command) override;
    void Shutdown() noexcept override;

    RendererState GetState() const noexcept override { return m_state; }
    bool IsInitialized() const noexcept override;
    uint32_t GetWidth() const noexcept override { return m_config.width; }
    uint32_t GetHeight() const noexcept override { return m_config.height; }
    const RendererConfig& GetConfig() const noexcept override { return m_config; }

    // D3D11-specific accessors for asset uploader and draw system.
    ID3D11Device* GetDevice() const noexcept { return m_device; }
    ID3D11DeviceContext* GetContext() const noexcept { return m_context; }
    IDXGISwapChain* GetSwapChain() const noexcept { return m_swapChain; }

    // Shader resource access.
    ID3D11VertexShader* GetVertexShader() const noexcept { return m_vertexShader; }
    ID3D11PixelShader* GetPixelShader() const noexcept { return m_pixelShader; }
    ID3D11InputLayout* GetInputLayout() const noexcept { return m_inputLayout; }
    ID3D11Buffer* GetConstantBuffer() const noexcept { return m_constantBuffer; }
    ID3D11SamplerState* GetSamplerState() const noexcept { return m_samplerState; }

    // Render target and depth stencil.
    ID3D11RenderTargetView* GetRenderTargetView() const noexcept { return m_renderTargetView; }
    ID3D11DepthStencilView* GetDepthStencilView() const noexcept { return m_depthStencilView; }

    // Viewport.
    const D3D11_VIEWPORT& GetViewport() const noexcept { return m_viewport; }

    // Attaches the asset uploader whose handles DrawIndexedMesh resolves.
    // Borrowed, never owned; the caller wires renderer -> uploader -> renderer
    // and must keep the uploader alive across draws. Required before any
    // DrawIndexedMesh call can succeed.
    void SetAssetUploader(D3D11AssetUploader* uploader) noexcept { m_assetUploader = uploader; }
    D3D11AssetUploader* GetAssetUploader() const noexcept { return m_assetUploader; }

private:
    // Resource creation helpers.
    Status CreateDeviceAndSwapChain(void* windowHandle);
    Status CreateRenderTargetView();
    Status CreateDepthStencilView();
    Status CreateShaders();
    Status CreateConstantBuffer();
    Status CreateSamplerState();
    Status CreateDefaultTexture();
    Status SetupViewport();

    // Cleanup helpers.
    void ReleaseResources();
    void ReleaseShaders();

    // State.
    RendererConfig m_config{};
    RendererState m_state = RendererState::Uninitialized;

    // D3D11 resources.
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;
    ID3D11RenderTargetView* m_renderTargetView = nullptr;
    ID3D11DepthStencilView* m_depthStencilView = nullptr;
    ID3D11Texture2D* m_depthStencilBuffer = nullptr;

    // Shaders and pipeline state.
    ID3D11VertexShader* m_vertexShader = nullptr;
    ID3D11PixelShader* m_pixelShader = nullptr;
    ID3D11InputLayout* m_inputLayout = nullptr;
    ID3D11Buffer* m_constantBuffer = nullptr;
    ID3D11SamplerState* m_samplerState = nullptr;

    // 1x1 white fallback texture for untextured draws.
    ID3D11ShaderResourceView* m_defaultTextureSrv = nullptr;

    // Viewport.
    D3D11_VIEWPORT m_viewport = {};

    // The uploader whose handles DrawIndexedMesh resolves. Borrowed.
    D3D11AssetUploader* m_assetUploader = nullptr;

    // Frame state.
    bool m_inFrame = false;
};

} // namespace Modern::Client