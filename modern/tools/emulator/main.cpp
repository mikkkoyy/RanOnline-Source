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

	return 0;
}
