#pragma once

#include "assets/ImageAsset.h"
#include "resources/ResourceData.h"
#include "types/Result.h"

namespace Modern::Client
{

// The byte -> typed image boundary (CLIENT-007).
//
// A decoder sees `ResourceData` and produces an `ImageAsset`. That is the whole
// contract, and it is deliberately narrow:
//
//   - **It does not know ResourceManager, IResourceProvider or ResourceId.** It
//     is handed bytes, not a resource, so it behaves identically whether those
//     bytes came from memory, a loose file, a future archive provider, or a
//     literal in a test.
//   - **It does not know IRenderer, and the renderer does not know it.** No GPU
//     object is created, nothing is uploaded, and no edge exists in either
//     direction between the asset layer and `modern/client/rendering`.
//   - **It does not know DirectX, Vulkan, OpenGL, MFC, Windows, or any legacy
//     RAN format.** A legacy texture would first be converted to modern bytes by
//     a future importer; decoding those bytes is this layer's job only after
//     that, and this layer never sees the legacy container.
//
// Decoders are **stateless by design**: no `Initialize()`, no `Shutdown()`, no
// configuration object, no state machine. A decoder acquires nothing — no
// device, no thread, no cache — so there is no lifetime to get wrong and no
// partial state for a failure to leave behind. A decoder that later needs
// configuration takes it as a constructor argument, and one that needs scratch
// space owns it as a member; neither turns into a lifecycle.
//
// The call is not `const`, so a decoder that keeps scratch space is not forced
// to declare it `mutable`. The provided decoder holds no state at all.
//
// Failures are returned by value and never thrown. The only code this boundary
// produces is `InvalidArgument`: bytes that are empty, truncated, malformed, of
// an unsupported version or layout, or whose geometry does not match their
// payload cannot be decoded, and no existing core code says that better than
// "the input was not acceptable".
class IImageDecoder
{
public:
	virtual ~IImageDecoder() = default;

	// Decodes an entire payload into a validated image.
	virtual Result<ImageAsset> DecodeImage(const ResourceData& data) = 0;
};

} // namespace Modern::Client
