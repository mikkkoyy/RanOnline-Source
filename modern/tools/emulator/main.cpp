// A small demonstration of the CORE-001 foundation.
//
// This used to be a 687-line harness for the legacy derived-stat port, which
// no longer exists in the core. What remains is a printout of the identity,
// result and lifecycle conventions, so the same facts can be eyeballed outside
// the test binary.
//
// The test suite in modern/tests/ is the authority; this executable exists only
// to show the API in use.

#include "character/Character.h"
#include "entity/Entity.h"
#include "item/ItemDefinition.h"
#include "item/ItemInstance.h"
#include "math/Vector3.h"
#include "types/Ids.h"
#include "types/Result.h"

#include "application/Application.h"
#include "input/FakeInputSource.h"
#include "input/InputEvents.h"
#include "input/InputSystem.h"

#include "assets/AssetTypes.h"
#include "assets/ImageAsset.h"
#include "assets/ImageDecoder.h"
#include "assets/TestImageDecoder.h"
#include "assets/DdsImageDecoder.h"
#include "assets/MeshAsset.h"
#include "assets/MeshDecoder.h"
#include "assets/TestMeshDecoder.h"

#include "rendering/AssetUpload.h"
#include "rendering/NullAssetUploader.h"
#include "rendering/NullRenderer.h"
#include "rendering/Renderer.h"
#include "rendering/RenderingTypes.h"

#include "resources/FileSystemResourceProvider.h"
#include "resources/MemoryResourceProvider.h"
#include "resources/ResourceData.h"
#include "resources/ResourceId.h"
#include "resources/ResourceManager.h"

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <string>
#include <vector>

using namespace Modern;

namespace
{
	void DescribeEntity(const char* label, const Entity<CharacterId>& entity)
	{
		const Vector3& position  = entity.GetPosition();
		const Vector3& direction = entity.GetDirection();

		std::printf(
			"  %-22s state=%-12s active=%-5s pos=(%.1f, %.1f, %.1f) dir=(%.2f, %.2f, %.2f)\n",
			label,
			ToString(entity.GetState()),
			entity.IsActive() ? "true" : "false",
			static_cast<double>(position.x),
			static_cast<double>(position.y),
			static_cast<double>(position.z),
			static_cast<double>(direction.x),
			static_cast<double>(direction.y),
			static_cast<double>(direction.z));
	}

	void ReportRejected(const char* what, const Status& status)
	{
		std::printf("  %-22s %s\n", what, ToString(status.GetCode()));
	}

	// Assembles the MIMG container described in assets/TestImageDecoder.h: a
	// ten-byte header followed by width * height * bytes per pixel of pixels.
	//
	// Built by hand because the container is documented rather than written by
	// a library routine, and because CLIENT-007 decodes no real image format:
	// showing a decoder in use must not imply one exists for RAN's textures.
	std::vector<uint8_t> MakeSampleImage(uint16_t width, uint16_t height, Modern::Client::ImageFormat format)
	{
		std::vector<uint8_t> bytes;
		bytes.push_back('M');
		bytes.push_back('I');
		bytes.push_back('M');
		bytes.push_back('G');
		bytes.push_back(Modern::Client::TestImageDecoder::kVersion);
		bytes.push_back(static_cast<uint8_t>(format));
		bytes.push_back(static_cast<uint8_t>(width & 0xFFu));
		bytes.push_back(static_cast<uint8_t>((width >> 8) & 0xFFu));
		bytes.push_back(static_cast<uint8_t>(height & 0xFFu));
		bytes.push_back(static_cast<uint8_t>((height >> 8) & 0xFFu));

		const size_t pixelBytes = static_cast<size_t>(width) * static_cast<size_t>(height) *
		                          static_cast<size_t>(Modern::Client::BytesPerPixel(format));
		for (size_t i = 0; i < pixelBytes; ++i)
		{
			bytes.push_back(static_cast<uint8_t>(i % 256u));
		}

		return bytes;
	}

	// Assembles the MMESH container described in assets/TestMeshDecoder.h: a
	// fourteen-byte header, then vertices as eight little endian float32
	// values each, then indices as little endian uint32. The sample is a unit
	// quad - four vertices, two triangles - so the counts and the triangle
	// count differ visibly in the printout.
	//
	// Built by hand for the same reasons as the image sample: the container is
	// documented rather than written by a library routine, and CLIENT-008
	// decodes no real mesh format, so showing a decoder in use must not imply
	// one exists for RAN's models.
	// Assembles a 4x4 DXT1 DDS file: the 128-byte header described in
	// assets/DdsImageDecoder.h, followed by one 8-byte colour block. The two
	// endpoints are pure red and pure blue, and every texel indexes the red
	// one, so a correct decoder produces a solid red 4x4.
	//
	// Built by hand for the same reason the MIMG and MMESH samples are: the
	// demo must not depend on a RAN asset being installed, and showing the
	// decoder in use must not imply it has read one.
	std::vector<uint8_t> MakeSampleDds()
	{
		std::vector<uint8_t> bytes;

		auto pushU16 = [](std::vector<uint8_t>& out, uint16_t value)
		{
			out.push_back(static_cast<uint8_t>(value & 0xFFu));
			out.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
		};
		auto pushU32 = [](std::vector<uint8_t>& out, uint32_t value)
		{
			out.push_back(static_cast<uint8_t>(value & 0xFFu));
			out.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
			out.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
			out.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
		};

		bytes.push_back('D');
		bytes.push_back('D');
		bytes.push_back('S');
		bytes.push_back(' ');
		pushU32(bytes, 124u);          // dwSize
		pushU32(bytes, 0x00001007u);   // caps | height | width | pixelformat
		pushU32(bytes, 4u);            // dwHeight
		pushU32(bytes, 4u);            // dwWidth
		pushU32(bytes, 8u);            // dwPitchOrLinearSize: one DXT1 block
		pushU32(bytes, 0u);            // dwDepth
		pushU32(bytes, 0u);            // dwMipMapCount
		for (int reserved = 0; reserved < 11; ++reserved)
		{
			pushU32(bytes, 0u);
		}

		pushU32(bytes, 32u);           // ddspf.dwSize
		pushU32(bytes, 0x4u);          // ddspf.dwFlags: DDPF_FOURCC
		pushU32(bytes, 0x31545844u);   // 'D','X','T','1'
		pushU32(bytes, 0u);            // dwRGBBitCount
		pushU32(bytes, 0u);            // masks
		pushU32(bytes, 0u);
		pushU32(bytes, 0u);
		pushU32(bytes, 0u);
		pushU32(bytes, 0x00001000u);   // dwCaps: texture
		pushU32(bytes, 0u);            // dwCaps2
		pushU32(bytes, 0u);            // dwCaps3
		pushU32(bytes, 0u);            // dwCaps4
		pushU32(bytes, 0u);            // dwReserved2

		pushU16(bytes, 0xF800u);       // colour0: red
		pushU16(bytes, 0x001Fu);       // colour1: blue
		pushU32(bytes, 0u);            // sixteen 2-bit indices, all zero

		return bytes;
	}

	std::vector<uint8_t> MakeSampleMesh()
	{
		auto pushU32 = [](std::vector<uint8_t>& bytes, uint32_t value)
		{
			bytes.push_back(static_cast<uint8_t>(value & 0xFFu));
			bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
			bytes.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
			bytes.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
		};
		auto pushF32 = [&pushU32](std::vector<uint8_t>& bytes, float value)
		{
			uint32_t bits = 0;
			std::memcpy(&bits, &value, sizeof(bits));
			pushU32(bytes, bits);
		};

		std::vector<uint8_t> bytes;
		bytes.push_back('M');
		bytes.push_back('E');
		bytes.push_back('S');
		bytes.push_back('H');
		bytes.push_back(Modern::Client::TestMeshDecoder::kVersion);
		bytes.push_back(static_cast<uint8_t>(Modern::Client::MeshVertexFormat::PositionNormalUvF32));
		pushU32(bytes, 4);  // vertex count
		pushU32(bytes, 6);  // index count: two whole triangles

		const float positions[4][3] = {
			{ -1.0f, -1.0f, 0.0f },
			{  1.0f, -1.0f, 0.0f },
			{  1.0f,  1.0f, 0.0f },
			{ -1.0f,  1.0f, 0.0f },
		};
		const float uvs[4][2] = {
			{ 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
		};
		for (size_t vertex = 0; vertex < 4; ++vertex)
		{
			pushF32(bytes, positions[vertex][0]);
			pushF32(bytes, positions[vertex][1]);
			pushF32(bytes, positions[vertex][2]);
			pushF32(bytes, 0.0f);  // normal x
			pushF32(bytes, 0.0f);  // normal y
			pushF32(bytes, 1.0f);  // normal z
			pushF32(bytes, uvs[vertex][0]);
			pushF32(bytes, uvs[vertex][1]);
		}

		const uint32_t indices[6] = { 0, 1, 2, 0, 2, 3 };
		for (const uint32_t index : indices)
		{
			pushU32(bytes, index);
		}

		return bytes;
	}
}

int main()
{
	const CharacterId characterId(static_cast<uint32_t>(42));

	std::printf("Identifiers\n");
	std::printf("  %-22s valid=%-5s value=%u\n",
		"CharacterId(42)", characterId.IsValid() ? "true" : "false", characterId.Get());
	std::printf("  %-22s valid=%-5s\n",
		"CharacterId()", CharacterId().IsValid() ? "true" : "false");

	std::printf("\nCreation\n");
	const Result<Character> created = Character::Create(characterId, "Demonstrator");
	if (created.IsError())
	{
		std::printf("  failed: %s\n", created.GetMessage());
		return 1;
	}

	Character character = created.GetValue();
	DescribeEntity("after Create", character);
	ReportRejected("empty name", Character::Create(characterId, "").GetStatus());
	ReportRejected("invalid id", Character::Create(CharacterId(), "Demonstrator").GetStatus());

	std::printf("\nLifecycle\n");
	ReportRejected("spawn before create", Character().Spawn(Vector3::Zero, Vector3::Forward));
	ReportRejected("zero direction", character.Spawn(Vector3::Zero, Vector3::Zero));
	ReportRejected("spawn", character.Spawn(Vector3(120.0f, 0.0f, -45.0f), Vector3::Right));
	DescribeEntity("after Spawn", character);
	ReportRejected("spawn again", character.Spawn(Vector3::Zero, Vector3::Forward));
	ReportRejected("despawn", character.Despawn());
	DescribeEntity("after Despawn", character);
	ReportRejected("despawn again", character.Despawn());
	ReportRejected("destroy", character.Destroy());
	DescribeEntity("after Destroy", character);
	ReportRejected("destroy again", character.Destroy());
	ReportRejected("set name", character.SetName("TooLate"));

	std::printf("\nCharacter state\n");
	character.Reset();

	// Reset returns the character to its default-constructed condition, so it
	// has to be created again before anything can be set on it.
	ReportRejected("set level after reset", character.SetLevel(12));
	character = Character::Create(characterId, "Demonstrator").GetValue();
	character.SetClass(CharacterClass::Swordsman);
	character.SetLevel(12);
	character.AddExperience(75000);
	std::printf("  %-22s class=%-10s level=%u experience=%lld\n",
		"after re-create + set", ToString(character.GetClass()),
		static_cast<unsigned>(character.GetLevel()),
		static_cast<long long>(character.GetExperience()));
	ReportRejected("level 0", character.SetLevel(0));
	ReportRejected("negative exp", character.AddExperience(-1));

	std::printf("\nItem identity\n");
	ItemDefinition definition;
	definition.id         = ItemId(static_cast<uint32_t>(1000));
	definition.kind       = ItemKind::Weapon;
	definition.name       = "Broadsword";
	definition.maxStack   = 1;
	std::printf("  %-22s valid=%-5s kind=%-10s name=%s\n",
		"ItemDefinition", definition.IsValid() ? "true" : "false",
		ToString(definition.kind), definition.name.c_str());

	ItemInstance instance;
	instance.definition = definition.id;
	instance.serial     = 1;
	instance.count      = 1;
	std::printf("  %-22s valid=%-5s free=%-5s serial=%llu\n",
		"ItemInstance", instance.IsValid() ? "true" : "false",
		instance.IsFree() ? "true" : "false",
		static_cast<unsigned long long>(instance.serial));

	std::printf("\nClient application, input & rendering\n");
	Modern::Client::ApplicationConfig appConfig;
	appConfig.maxFrames = 2;
	Modern::Client::Application app(appConfig);

	Modern::Client::InputSystem input;
	input.Initialize();
	app.SetInputSystem(&input);

	Modern::Client::NullRenderer renderer;
	Modern::Client::RendererConfig renConfig{ 1280, 720, Modern::Client::DisplayMode::Windowed, true };
	const auto renStatus = renderer.Initialize(renConfig);
	std::printf("  %-22s %s\n", "renderer init", renStatus.GetMessage());
	app.SetRenderer(&renderer);

	std::vector<Modern::Client::InputEvent> script;
	script.push_back(Modern::Client::InputEvent::MakeKey(Modern::Client::KeyCode::W, true));
	script.push_back(Modern::Client::InputEvent::MakeMouseMove(100, 200));
	app.SetInputSource(std::make_unique<Modern::Client::ScriptedInputSource>(std::move(script)));

	app.SubscribeInput([](const Modern::Client::InputEvent& ev)
	{
		std::printf("  %-22s event=%s\n", "input event", Modern::Client::ToString(ev.type));
	});

	std::printf("  %-22s state=%s\n", "created", Modern::Client::ToString(app.GetState()));
	std::printf("  %-22s %s\n", "initialize", app.Initialize().GetMessage());
	std::printf("  %-22s state=%s\n", "initialized", Modern::Client::ToString(app.GetState()));

	app.SetUpdateCallback([&input](uint64_t frame)
	{
		std::printf("  %-22s frame=%llu KeyW=%s Mouse=(%d,%d)\n",
			"update",
			static_cast<unsigned long long>(frame),
			input.IsKeyDown(Modern::Client::KeyCode::W) ? "down" : "up",
			input.GetMouseX(),
			input.GetMouseY());
	});

	app.SetRenderCallback([&renderer](uint64_t frame)
	{
		std::printf("  %-22s frame=%llu state=%s\n",
			"render callback",
			static_cast<unsigned long long>(frame),
			Modern::Client::ToString(renderer.GetState()));
		renderer.Clear(Modern::Client::RenderColor::ClearCornflowerBlue);
	});

	std::printf("  %-22s %s\n", "run", app.Run().GetMessage());
	std::printf("  %-22s state=%s frames=%llu\n",
		"stopped", Modern::Client::ToString(app.GetState()),
		static_cast<unsigned long long>(app.GetFrameCount()));
	std::printf("  %-22s rendered=%llu clears=%llu\n",
		"renderer stats",
		static_cast<unsigned long long>(renderer.GetRenderedFrameCount()),
		static_cast<unsigned long long>(renderer.GetClearCount()));
	renderer.Shutdown();
	std::printf("  %-22s state=%s\n", "renderer shutdown", Modern::Client::ToString(renderer.GetState()));


	std::printf("\nClient resource management\n");
	Modern::Client::MemoryResourceProvider resourceProvider;
	Modern::Client::ResourceManager resourceManager;
	resourceManager.SetProvider(&resourceProvider);
	std::printf("  %-22s %s\n", "resource mgr init", resourceManager.Initialize().GetMessage());

	auto resId = Modern::Client::ResourceId::Create("ui/textures/login_background").GetValue();
	Modern::Client::ResourceData mockTexture("SAMPLE_TEXTURE_BINARY_DATA");
	resourceProvider.RegisterResource(resId, mockTexture);
	std::printf("  %-22s id=%s registered\n", "provider", resId.GetName().c_str());

	auto loaded = resourceManager.Load(resId);
	if (loaded.IsOk())
	{
		std::printf("  %-22s id=%s bytes=%zu cached=%s\n",
			"resource loaded",
			resId.GetName().c_str(),
			loaded.GetValue().GetSize(),
			resourceManager.IsCached(resId) ? "true" : "false");
	}
	resourceManager.Shutdown();
	std::printf("  %-22s state=%s\n", "resource mgr shutdown", Modern::Client::ToString(resourceManager.GetState()));

	std::printf("\nClient filesystem resource provider\n");
	{
		// A root created and removed here on purpose. The emulator has no
		// asset directory of its own and must not grow one: pointing it at a
		// developer's RAN installation would make this harness depend on a
		// machine, and the point of the provider is that it depends on none.
		std::error_code ec;
		const std::filesystem::path demoRoot =
			std::filesystem::temp_directory_path(ec) / "modern_emulator_resource_demo";
		std::filesystem::remove_all(demoRoot, ec);
		std::filesystem::create_directories(demoRoot / "ui", ec);

		{
			std::ofstream out(demoRoot / "ui" / "login.bin", std::ios::binary | std::ios::trunc);
			out << "EMULATOR_FILE_RESOURCE";
		}

		Modern::Client::FileSystemResourceProvider fsProvider(demoRoot);
		Modern::Client::ResourceManager fsManager;
		fsManager.SetProvider(&fsProvider);

		std::printf("  %-22s %s\n", "fs provider init", fsProvider.Initialize().GetMessage());
		std::printf("  %-22s %s\n", "fs manager init", fsManager.Initialize().GetMessage());

		const auto fsId = Modern::Client::ResourceId::Create("ui/login.bin");
		if (fsId.IsOk())
		{
			const auto fsData = fsManager.Load(fsId.GetValue());
			if (fsData.IsOk())
			{
				std::printf("  %-22s id=%s bytes=%zu cached=%s\n",
					"fs resource loaded",
					fsId.GetValue().GetName().c_str(),
					fsData.GetValue().GetSize(),
					fsManager.IsCached(fsId.GetValue()) ? "true" : "false");
			}
		}

		// The provider refuses to leave its root, and says so with a code
		// rather than an exception.
		const auto escapeId = Modern::Client::ResourceId::Create("../secret.bin");
		if (escapeId.IsOk())
		{
			std::printf("  %-22s %s\n", "traversal rejected",
				Modern::ToString(fsProvider.Load(escapeId.GetValue()).GetError()));
		}

		fsManager.Shutdown();
		std::printf("  %-22s state=%s\n", "fs manager shutdown",
			Modern::Client::ToString(fsManager.GetState()));

		std::filesystem::remove_all(demoRoot, ec);
	}

	std::printf("\nClient typed asset decoder\n");
	{
		// A decoder is stateless: nothing is acquired, so there is no
		// Initialize() to call and no Shutdown() to forget.
		Modern::Client::TestImageDecoder decoder;
		std::printf("  %-22s stateless\n", "decoder lifetime");

		// Deterministic in-memory bytes: the emulator has no asset directory of
		// its own and must not grow one, and CLIENT-007 ships no decoder for a
		// real image format, so the sample is assembled rather than loaded.
		const std::vector<uint8_t> sample =
			MakeSampleImage(3, 2, Modern::Client::ImageFormat::R8G8B8A8_UNorm);

		const auto decoded = decoder.DecodeImage(Modern::Client::ResourceData(sample));
		if (decoded.IsOk())
		{
			const Modern::Client::ImageAsset& image = decoded.GetValue();
			std::printf("  %-22s size=%ux%u format=%s\n",
				"test image decoded",
				image.GetWidth(),
				image.GetHeight(),
				Modern::Client::ToString(image.GetFormat()));
			std::printf("  %-22s payload=%zu bytes (%zu pixels)\n",
				"image metadata",
				image.GetPixelByteCount(),
				image.GetPixelCount());
		}

		// Malformed bytes are refused with a code rather than an exception, and
		// the next call is unaffected: there is no broken state to recover from.
		std::vector<uint8_t> corrupted = sample;
		corrupted[0] = 'X';
		std::printf("  %-22s %s\n",
			"malformed rejected",
			Modern::ToString(decoder.DecodeImage(Modern::Client::ResourceData(corrupted)).GetError()));

		// provider -> ResourceManager -> ResourceData -> decoder -> asset, the
		// whole CLIENT-007 path in one place.
		Modern::Client::MemoryResourceProvider assetProvider;
		Modern::Client::ResourceManager assetManager;
		assetManager.SetProvider(&assetProvider);
		std::printf("  %-22s %s\n", "asset manager init", assetManager.Initialize().GetMessage());

		const auto assetId = Modern::Client::ResourceId::Create("ui/test_image.mimg");
		if (assetId.IsOk())
		{
			assetProvider.RegisterResource(assetId.GetValue(), Modern::Client::ResourceData(sample));

			const auto loadedBytes = assetManager.Load(assetId.GetValue());
			if (loadedBytes.IsOk())
			{
				const auto fromManager = decoder.DecodeImage(loadedBytes.GetValue());
				if (fromManager.IsOk())
				{
					std::printf("  %-22s id=%s bytes=%zu cached=%s\n",
						"resource decoded",
						assetId.GetValue().GetName().c_str(),
						fromManager.GetValue().GetPixelByteCount(),
						assetManager.IsCached(assetId.GetValue()) ? "true" : "false");
				}
			}
		}

		assetManager.Shutdown();
		std::printf("  %-22s state=%s\n",
			"asset manager shutdown",
			Modern::Client::ToString(assetManager.GetState()));
	}

	std::printf("\nClient typed mesh decoder\n");
	{
		// The geometry half of the same boundary: stateless again, with its
		// own interface and its own asset type rather than a second face on
		// the image ones.
		Modern::Client::TestMeshDecoder meshDecoder;
		std::printf("  %-22s stateless\n", "decoder lifetime");

		// Deterministic in-memory bytes, assembled by hand: CLIENT-008 ships
		// no decoder for a real mesh format either, so the sample is built
		// from the documented test container rather than loaded.
		const std::vector<uint8_t> sample = MakeSampleMesh();

		const auto decodedMesh = meshDecoder.DecodeMesh(Modern::Client::ResourceData(sample));
		if (decodedMesh.IsOk())
		{
			const Modern::Client::MeshAsset& mesh = decodedMesh.GetValue();
			std::printf("  %-22s topology=%s triangles=%zu\n",
				"test mesh decoded",
				Modern::Client::ToString(mesh.GetTopology()),
				mesh.GetTriangleCount());
			std::printf("  %-22s vertices=%zu indices=%zu bytes=%zu\n",
				"mesh metadata",
				mesh.GetVertexCount(),
				mesh.GetIndexCount(),
				mesh.GetTotalByteCount());
		}

		// Malformed bytes are refused with a code rather than an exception,
		// and the next call is unaffected - no broken state to recover from.
		std::vector<uint8_t> corrupted = sample;
		corrupted[0] = 'X';
		std::printf("  %-22s %s\n",
			"malformed rejected",
			Modern::ToString(meshDecoder.DecodeMesh(Modern::Client::ResourceData(corrupted)).GetError()));

		// A structurally perfect container whose geometry is wrong: the last
		// index names a vertex that does not exist. The header and the size
		// checks both pass, and construction is where it is caught - the part
		// of the boundary that no length field can see.
		std::vector<uint8_t> badIndex = sample;
		const size_t lastIndexOffset = Modern::Client::TestMeshDecoder::kHeaderSize +
			4 * Modern::Client::kMeshVertexBytes + 5 * Modern::Client::kMeshIndexBytes;
		badIndex[lastIndexOffset + 0] = 99;
		std::printf("  %-22s %s\n",
			"index out of range",
			Modern::ToString(meshDecoder.DecodeMesh(Modern::Client::ResourceData(badIndex)).GetError()));

		// provider -> ResourceManager -> ResourceData -> decoder -> MeshAsset,
		// the whole CLIENT-008 path in one place, over the same manager the
		// image section used.
		Modern::Client::MemoryResourceProvider meshProvider;
		Modern::Client::ResourceManager        meshManager;
		meshManager.SetProvider(&meshProvider);
		std::printf("  %-22s %s\n", "mesh manager init", meshManager.Initialize().GetMessage());

		const auto meshId = Modern::Client::ResourceId::Create("world/test_quad.mmsh");
		if (meshId.IsOk())
		{
			meshProvider.RegisterResource(meshId.GetValue(), Modern::Client::ResourceData(sample));

			const auto loadedMeshBytes = meshManager.Load(meshId.GetValue());
			if (loadedMeshBytes.IsOk())
			{
				const auto meshFromManager = meshDecoder.DecodeMesh(loadedMeshBytes.GetValue());
				if (meshFromManager.IsOk())
				{
					std::printf("  %-22s id=%s triangles=%zu cached=%s\n",
						"mesh resource decoded",
						meshId.GetValue().GetName().c_str(),
						meshFromManager.GetValue().GetTriangleCount(),
						meshManager.IsCached(meshId.GetValue()) ? "true" : "false");
				}
			}
		}

		meshManager.Shutdown();
		std::printf("  %-22s state=%s\n",
			"mesh manager shutdown",
			Modern::Client::ToString(meshManager.GetState()));
	}

	std::printf("\nClient renderer asset boundary\n");
	{
		// CLIENT-009 in miniature: the two decoded CPU assets go in,
		// deterministic opaque handles come out, and nothing GPU, platform
		// or filesystem is touched. The renderer's state is the uploader's
		// whole lifecycle -- there is no second Initialize/Shutdown pair,
		// and the uploader retains metadata only, never asset bytes.
		Modern::Client::TestImageDecoder imageDecoder;
		Modern::Client::TestMeshDecoder  meshDecoder;

		const auto uploadImageAsset = imageDecoder.DecodeImage(Modern::Client::ResourceData(
			MakeSampleImage(4, 2, Modern::Client::ImageFormat::R8G8B8A8_UNorm)));
		const auto uploadMeshAsset =
			meshDecoder.DecodeMesh(Modern::Client::ResourceData(MakeSampleMesh()));

		Modern::Client::NullRenderer uploadRenderer;
		Modern::Client::NullAssetUploader uploader(&uploadRenderer);

		// Before the renderer exists: the boundary refuses with the same
		// code IRenderer itself would use.
		if (uploadImageAsset.IsOk())
		{
			std::printf("  %-22s %s\n", "upload before init",
				Modern::ToString(uploader.UploadImage(uploadImageAsset.GetValue()).GetError()));
		}

		const Modern::Client::RendererConfig uploadConfig{ 1, 1, Modern::Client::DisplayMode::Windowed, true };
		std::printf("  %-22s %s\n", "renderer init",
			uploadRenderer.Initialize(uploadConfig).GetMessage());

		Result<Modern::Client::ImageResourceHandle> imageHandle =
			Status(ErrorCode::InvalidState);
		if (uploadImageAsset.IsOk())
		{
			imageHandle = uploader.UploadImage(uploadImageAsset.GetValue());
			if (imageHandle.IsOk())
			{
				std::printf("  %-22s handle=%llu\n", "image uploaded",
					static_cast<unsigned long long>(imageHandle.GetValue().Get()));
				const auto imageInfo = uploader.GetImageInfo(imageHandle.GetValue());
				if (imageInfo.IsOk())
				{
					std::printf("  %-22s %ux%u %s bytes=%zu\n", "image metadata",
						imageInfo.GetValue().width,
						imageInfo.GetValue().height,
						Modern::Client::ToString(imageInfo.GetValue().format),
						imageInfo.GetValue().byteCount);
				}
			}
		}

		Result<Modern::Client::MeshResourceHandle> meshHandle =
			Status(ErrorCode::InvalidState);
		if (uploadMeshAsset.IsOk())
		{
			meshHandle = uploader.UploadMesh(uploadMeshAsset.GetValue());
			if (meshHandle.IsOk())
			{
				std::printf("  %-22s handle=%llu\n", "mesh uploaded",
					static_cast<unsigned long long>(meshHandle.GetValue().Get()));
				const auto meshInfo = uploader.GetMeshInfo(meshHandle.GetValue());
				if (meshInfo.IsOk())
				{
					std::printf("  %-22s vertices=%zu indices=%zu triangles=%zu topology=%s\n",
						"mesh metadata",
						meshInfo.GetValue().vertexCount,
						meshInfo.GetValue().indexCount,
						meshInfo.GetValue().triangleCount,
						Modern::Client::ToString(meshInfo.GetValue().topology));
				}
			}
		}

		// Releases reach only the uploader's registry; the storage layer
		// above never learns a handle existed.
		if (imageHandle.IsOk())
		{
			uploader.ReleaseImage(imageHandle.GetValue());
			std::printf("  %-22s handle=%llu live=%zu\n", "image released",
				static_cast<unsigned long long>(imageHandle.GetValue().Get()),
				uploader.GetLiveImageCount());
		}
		if (meshHandle.IsOk())
		{
			uploader.ReleaseMesh(meshHandle.GetValue());
			std::printf("  %-22s handle=%llu live=%zu\n", "mesh released",
				static_cast<unsigned long long>(meshHandle.GetValue().Get()),
				uploader.GetLiveMeshCount());
		}

		// Shutdown while a handle exists: the handle dies with the renderer
		// and the next upload says so with the terminal-state code.
		if (uploadImageAsset.IsOk())
		{
			const auto late = uploader.UploadImage(uploadImageAsset.GetValue());
			uploadRenderer.Shutdown();
			std::printf("  %-22s %s\n", "upload after shutdown",
				Modern::ToString(uploader.UploadImage(uploadImageAsset.GetValue()).GetError()));
			if (late.IsOk())
			{
				std::printf("  %-22s %s\n", "handle after shutdown",
					uploader.IsValidImage(late.GetValue()) ? "valid" : "invalid");
			}
		}
	}

	std::printf("\nClient real DDS decoder\n");
	{
		// CLIENT-011: a real shipped format through the real decoder. The
		// bytes are a 4x4 DXT1 file assembled above, so the demo needs no RAN
		// installation -- and reading a hand-built buffer is not a claim of
		// having read a RAN asset, which the test suite checks separately
		// against real files when one is available.
		Modern::Client::DdsImageDecoder decoder;
		const std::vector<uint8_t> sample = MakeSampleDds();

		const auto decoded = decoder.DecodeImage(Modern::Client::ResourceData(sample));
		std::printf("  %-22s %s\n", "dds decoder", "stateless");

		if (decoded.IsOk())
		{
			const Modern::Client::ImageAsset& image = decoded.GetValue();
			const std::vector<uint8_t>& pixels = image.GetPixels();
			std::printf("  %-22s %ux%u %s bytes=%zu\n", "dds decoded",
				image.GetWidth(),
				image.GetHeight(),
				Modern::Client::ToString(image.GetFormat()),
				image.GetPixelByteCount());

			// The first texel proves the block was decoded, not merely
			// accepted: the c0 endpoint was pure red and every index was 0.
			std::printf("  %-22s (%u, %u, %u, %u)\n", "first texel",
				pixels[0], pixels[1], pixels[2], pixels[3]);

			// And the same asset crosses the CLIENT-009 upload boundary, so
			// the two milestones meet exactly where the architecture says.
			Modern::Client::NullRenderer ddsRenderer;
			Modern::Client::NullAssetUploader ddsUploader(&ddsRenderer);
			const Modern::Client::RendererConfig ddsConfig{ 1, 1, Modern::Client::DisplayMode::Windowed, true };
			ddsRenderer.Initialize(ddsConfig);

			const auto handle = ddsUploader.UploadImage(image);
			if (handle.IsOk())
			{
				std::printf("  %-22s handle=%llu\n", "dds uploaded",
					static_cast<unsigned long long>(handle.GetValue().Get()));
				const auto info = ddsUploader.GetImageInfo(handle.GetValue());
				if (info.IsOk())
				{
					std::printf("  %-22s %ux%u %s bytes=%zu\n", "resource metadata",
						info.GetValue().width, info.GetValue().height,
						Modern::Client::ToString(info.GetValue().format),
						info.GetValue().byteCount);
				}
				std::printf("  %-22s %s\n", "dds released",
					Modern::ToString(ddsUploader.ReleaseImage(handle.GetValue()).GetCode()));
			}
		}
		else
		{
			std::printf("  %-22s %s\n", "dds decoded", Modern::ToString(decoded.GetError()));
		}

		// A truncated header is refused rather than half-read, which is the
		// property that matters for untrusted bytes.
		const std::vector<uint8_t> cut(sample.begin(), sample.begin() + 40);
		std::printf("  %-22s %s\n", "truncated dds",
			Modern::ToString(decoder.DecodeImage(Modern::Client::ResourceData(cut)).GetError()));
	}

	return 0;
}
