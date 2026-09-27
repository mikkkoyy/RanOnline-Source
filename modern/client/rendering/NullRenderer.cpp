#include "NullRenderer.h"

namespace Modern::Client
{

NullRenderer::~NullRenderer()
{
	Shutdown();
}

Status NullRenderer::Initialize(const RendererConfig& config)
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

	m_config             = config;
	m_state              = RendererState::Initialized;
	m_renderedFrameCount = 0;
	m_clearCount         = 0;

	return Ok();
}

Status NullRenderer::BeginFrame()
{
	if (m_state == RendererState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != RendererState::Initialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	m_state = RendererState::InFrame;
	return Ok();
}

Status NullRenderer::EndFrame()
{
	if (m_state == RendererState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != RendererState::InFrame)
	{
		return Status(ErrorCode::InvalidState);
	}

	m_state = RendererState::Initialized;
	++m_renderedFrameCount;
	return Ok();
}

Status NullRenderer::Clear(const RenderColor& color)
{
	if (m_state == RendererState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != RendererState::InFrame)
	{
		return Status(ErrorCode::InvalidState);
	}

	m_lastClearColor = color;
	++m_clearCount;
	return Ok();
}

Status NullRenderer::Resize(uint32_t width, uint32_t height)
{
	if (m_state == RendererState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != RendererState::Initialized)
	{
		// Cannot resize while InFrame or Uninitialized
		return Status(ErrorCode::InvalidState);
	}

	if (width == 0 || height == 0)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	m_config.width  = width;
	m_config.height = height;
	return Ok();
}

void NullRenderer::Shutdown() noexcept
{
	m_state = RendererState::ShutdownState;
}

bool NullRenderer::IsInitialized() const noexcept
{
	return m_state == RendererState::Initialized || m_state == RendererState::InFrame;
}

} // namespace Modern::Client

