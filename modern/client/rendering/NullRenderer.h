#pragma once

// CLIENT-004: deterministic headless null renderer.
//
// Fully implements IRenderer without allocating GPU resources, creating a
// device, or interacting with the OS windowing system.
//
// Tracks state transitions, frame counts, clear operations, and dimension
// updates for unit tests, headless emulation, and CI.

#include "Renderer.h"

namespace Modern::Client
{

class NullRenderer final : public IRenderer
{
public:
	NullRenderer() = default;
	~NullRenderer() override;

	// IRenderer implementation
	Status Initialize(const RendererConfig& config) override;
	Status BeginFrame() override;
	Status EndFrame() override;
	Status Clear(const RenderColor& color) override;
	Status Resize(uint32_t width, uint32_t height) override;
	void Shutdown() noexcept override;

	RendererState GetState() const noexcept override { return m_state; }
	bool IsInitialized() const noexcept override;
	uint32_t GetWidth() const noexcept override { return m_config.width; }
	uint32_t GetHeight() const noexcept override { return m_config.height; }
	const RendererConfig& GetConfig() const noexcept override { return m_config; }

	// Test/inspection queries
	uint64_t GetRenderedFrameCount() const noexcept { return m_renderedFrameCount; }
	uint64_t GetClearCount() const noexcept { return m_clearCount; }
	const RenderColor& GetLastClearColor() const noexcept { return m_lastClearColor; }

private:
	RendererConfig m_config;
	RendererState  m_state              = RendererState::Uninitialized;
	uint64_t       m_renderedFrameCount = 0;
	uint64_t       m_clearCount         = 0;
	RenderColor    m_lastClearColor;
};

} // namespace Modern::Client
