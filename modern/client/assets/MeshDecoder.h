#pragma once

#include "assets/MeshAsset.h"
#include "resources/ResourceData.h"
#include "types/Result.h"

namespace Modern::Client
{

// The byte -> typed mesh boundary (CLIENT-008).
//
// A decoder sees `ResourceData` and produces a `MeshAsset`. That is the whole
// contract, and it is deliberately narrow in exactly the ways IImageDecoder
// (CLIENT-007) is narrow, because the two boundaries have to be interchangeable
// in shape if the asset layer is going to hold more than one asset type:
//
//   - **It does not know ResourceManager, IResourceProvider or ResourceId.** It
//     is handed bytes, not a resource, so it behaves identically whether those
//     bytes came from memory, a loose file, a future archive provider, or a
//     literal in a test.
//   - **It does not know IRenderer, and the renderer does not know it.** No GPU
//     object is created, no vertex or index buffer is allocated, nothing is
//     uploaded, and no edge exists in either direction between the asset layer
//     and `modern/client/rendering`.
//   - **It does not know DirectX, Vulkan, OpenGL, MFC, Windows, or any legacy
//     RAN format.** A legacy mesh would first be converted to modern bytes by a
//     future importer; decoding those bytes is this layer's job only after
//     that, and this layer never sees the legacy container.
//
// Decoders are **stateless by design**, the same contract as IImageDecoder: no
// `Initialize()`, no `Shutdown()`, no configuration object, no cache, no
// thread. A failure is returned by value and never thrown. The only code this
// boundary produces is `InvalidArgument`: bytes that are empty, truncated,
// malformed, of an unsupported version or vertex layout, whose geometry exceeds
// the ceilings, or whose indices and floats do not describe a real mesh cannot
// be decoded, and no existing core code says that better than "the input was
// not acceptable". `NotFound` stays what the layer below reports, so "the
// resource is not there" and "the resource is not a mesh" never collapse into
// one code.
//
// The call is not `const`, so a decoder that keeps scratch space is not forced
// to declare it `mutable`. The provided decoder holds no state at all.
class IMeshDecoder
{
public:
	virtual ~IMeshDecoder() = default;

	// Decodes an entire payload into a validated mesh.
	virtual Result<MeshAsset> DecodeMesh(const ResourceData& data) = 0;
};

} // namespace Modern::Client