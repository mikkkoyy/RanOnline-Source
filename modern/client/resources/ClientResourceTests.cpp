// CLIENT-005: modern client resource / asset boundary tests.
//
// Covers:
// 1. ResourceId rejects invalid/empty identifiers.
// 2. Valid ResourceId compares correctly.
// 3. ResourceData preserves byte contents.
// 4. MemoryResourceProvider can register a resource.
// 5. Existing resource can be loaded.
// 6. Missing resource returns NotFound.
// 7. Duplicate registration is deterministic.
// 8. Repeated lookup is deterministic.
// 9. ResourceManager uses its injected provider.
// 10. ResourceManager does not require a renderer.
// 11. ResourceManager can operate headlessly.
// 12. Shutdown/lifecycle behavior is deterministic.
// 13. Invalid operations return errors rather than throwing.
// 14. No legacy headers are required.

#include "TestHarness.h"

#include "resources/MemoryResourceProvider.h"
#include "resources/ResourceData.h"
#include "resources/ResourceId.h"
#include "resources/ResourceManager.h"
#include "resources/ResourceProvider.h"
#include "types/Result.h"

#include <cstdint>
#include <string>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

// ---------------------------------------------------------------------------
// 1 & 2: ResourceId validation and comparison
// ---------------------------------------------------------------------------

MODERN_TEST(ClientResources_ResourceIdRejectsInvalidOrEmpty)
{
	auto res1 = ResourceId::Create("");
	CHECK(res1.IsError());
	CHECK_EQ(res1.GetError(), ErrorCode::InvalidArgument);

	auto res2 = ResourceId::Create("   \t\n");
	CHECK(res2.IsError());
	CHECK_EQ(res2.GetError(), ErrorCode::InvalidArgument);

	auto res3 = ResourceId::Create("textures\\ui\\login");
	CHECK(res3.IsError());
	CHECK_EQ(res3.GetError(), ErrorCode::InvalidArgument);

	auto res4 = ResourceId::Create("/textures/ui/login");
	CHECK(res4.IsError());
	CHECK_EQ(res4.GetError(), ErrorCode::InvalidArgument);

	auto res5 = ResourceId::Create("textures/ui/login/");
	CHECK(res5.IsError());
	CHECK_EQ(res5.GetError(), ErrorCode::InvalidArgument);

	ResourceId defaultId;
	CHECK(!defaultId.IsValid());
}

MODERN_TEST(ClientResources_ValidResourceIdComparesCorrectly)
{
	auto res1 = ResourceId::Create("textures/ui/login");
	auto res2 = ResourceId::Create("textures/ui/login");
	auto res3 = ResourceId::Create("textures/ui/char_select");

	CHECK(res1.IsOk());
	CHECK(res2.IsOk());
	CHECK(res3.IsOk());

	ResourceId id1 = res1.GetValue();
	ResourceId id2 = res2.GetValue();
	ResourceId id3 = res3.GetValue();

	CHECK(id1.IsValid());
	CHECK_EQ(id1, id2);
	CHECK(id1 != id3);
	CHECK(id3 < id1);

	CHECK_EQ(id1.GetName(), "textures/ui/login");
	CHECK_EQ(id1.GetView(), "textures/ui/login");
}

// ---------------------------------------------------------------------------
// 3: ResourceData preserves byte contents
// ---------------------------------------------------------------------------

MODERN_TEST(ClientResources_ResourceDataPreservesByteContents)
{
	const std::vector<uint8_t> raw = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03 };
	ResourceData data(raw);

	CHECK(!data.IsEmpty());
	CHECK_EQ(data.GetSize(), raw.size());
	CHECK(data.GetBytes() == raw);
	CHECK_EQ(data.GetData()[0], 0xDE);
	CHECK_EQ(data.GetData()[3], 0xEF);

	std::string text = "RAN_ONLINE_TEST_PAYLOAD";
	ResourceData textData(text);
	CHECK_EQ(textData.GetSize(), text.size());
	CHECK_EQ(textData.AsStringView(), text);

	ResourceData textData2(text);
	CHECK_EQ(textData, textData2);
	CHECK(data != textData);

	ResourceData emptyData;
	CHECK(emptyData.IsEmpty());
	CHECK_EQ(emptyData.GetSize(), 0u);
}

// ---------------------------------------------------------------------------
// 4, 5, 6, 7: MemoryResourceProvider functionality
// ---------------------------------------------------------------------------

MODERN_TEST(ClientResources_MemoryProviderRegisterAndLoadExisting)
{
	MemoryResourceProvider provider;
	CHECK_EQ(provider.GetCount(), 0u);

	auto idRes = ResourceId::Create("test/mock_data");
	CHECK(idRes.IsOk());
	ResourceId id = idRes.GetValue();

	CHECK(!provider.HasResource(id));

	std::string payload = "binary_mock_contents";
	ResourceData data(payload);

	Status regStatus = provider.RegisterResource(id, data);
	CHECK(regStatus.IsOk());
	CHECK_EQ(provider.GetCount(), 1u);
	CHECK(provider.HasResource(id));

	auto loadRes = provider.Load(id);
	CHECK(loadRes.IsOk());
	CHECK_EQ(loadRes.GetValue().AsStringView(), payload);
}

MODERN_TEST(ClientResources_MemoryProviderMissingReturnsNotFound)
{
	MemoryResourceProvider provider;

	auto idRes = ResourceId::Create("missing/resource");
	CHECK(idRes.IsOk());
	ResourceId id = idRes.GetValue();

	CHECK(!provider.HasResource(id));
	auto loadRes = provider.Load(id);
	CHECK(loadRes.IsError());
	CHECK_EQ(loadRes.GetError(), ErrorCode::NotFound);

	Status unregStatus = provider.UnregisterResource(id);
	CHECK(unregStatus.IsError());
	CHECK_EQ(unregStatus.GetCode(), ErrorCode::NotFound);
}

MODERN_TEST(ClientResources_MemoryProviderDuplicateRegistrationIsDeterministic)
{
	MemoryResourceProvider provider;
	auto id = ResourceId::Create("config/settings").GetValue();

	ResourceData v1("version_1");
	ResourceData v2("version_2");

	CHECK(provider.RegisterResource(id, v1).IsOk());
	CHECK_EQ(provider.Load(id).GetValue().AsStringView(), "version_1");

	CHECK(provider.RegisterResource(id, v2).IsOk());
	CHECK_EQ(provider.GetCount(), 1u);
	CHECK_EQ(provider.Load(id).GetValue().AsStringView(), "version_2");

	CHECK(provider.UnregisterResource(id).IsOk());
	CHECK_EQ(provider.GetCount(), 0u);
	CHECK(!provider.HasResource(id));
}

// ---------------------------------------------------------------------------
// 8, 9, 10, 11: ResourceManager Coordination, Cache, and Headless Execution
// ---------------------------------------------------------------------------

MODERN_TEST(ClientResources_ResourceManagerUsesInjectedProviderAndCaches)
{
	MemoryResourceProvider provider;
	auto idA = ResourceId::Create("textures/terrain").GetValue();
	auto idB = ResourceId::Create("textures/water").GetValue();

	provider.RegisterResource(idA, ResourceData("TERRAIN_MAP"));
	provider.RegisterResource(idB, ResourceData("WATER_MAP"));

	ResourceManager manager;
	CHECK_EQ(manager.GetState(), ResourceManagerState::Uninitialized);

	manager.SetProvider(&provider);
	CHECK_EQ(manager.GetProvider(), &provider);

	Status initStatus = manager.Initialize();
	CHECK(initStatus.IsOk());
	CHECK_EQ(manager.GetState(), ResourceManagerState::Ready);

	CHECK_EQ(manager.GetCachedCount(), 0u);
	CHECK(!manager.IsCached(idA));
	CHECK(!manager.IsCached(idB));

	auto loadA = manager.Load(idA);
	CHECK(loadA.IsOk());
	CHECK_EQ(loadA.GetValue().AsStringView(), "TERRAIN_MAP");
	CHECK_EQ(manager.GetCachedCount(), 1u);
	CHECK(manager.IsCached(idA));

	auto loadA2 = manager.Load(idA);
	CHECK(loadA2.IsOk());
	CHECK_EQ(loadA2.GetValue().AsStringView(), "TERRAIN_MAP");
	CHECK_EQ(manager.GetCachedCount(), 1u);

	auto loadB = manager.Load(idB);
	CHECK(loadB.IsOk());
	CHECK_EQ(loadB.GetValue().AsStringView(), "WATER_MAP");
	CHECK_EQ(manager.GetCachedCount(), 2u);
	CHECK(manager.IsCached(idB));

	CHECK(manager.ClearCache().IsOk());
	CHECK_EQ(manager.GetCachedCount(), 0u);
	CHECK(!manager.IsCached(idA));
	CHECK(!manager.IsCached(idB));
}

MODERN_TEST(ClientResources_ResourceManagerHeadlessWithoutRenderer)
{
	MemoryResourceProvider provider;
	auto id = ResourceId::Create("scripts/dialogue").GetValue();
	provider.RegisterResource(id, ResourceData("DIALOGUE_TREE_DATA"));

	ResourceManager manager;
	manager.SetProvider(&provider);
	CHECK(manager.Initialize().IsOk());

	auto result = manager.Load(id);
	CHECK(result.IsOk());
	CHECK_EQ(result.GetValue().AsStringView(), "DIALOGUE_TREE_DATA");
	CHECK(manager.Shutdown().IsOk());
}

// ---------------------------------------------------------------------------
// 12 & 13: Lifecycle and Error Handling without Throwing
// ---------------------------------------------------------------------------

MODERN_TEST(ClientResources_ResourceManagerLifecycleTransitions)
{
	ResourceManager manager;
	CHECK_EQ(manager.GetState(), ResourceManagerState::Uninitialized);
	CHECK_EQ(std::string(ToString(manager.GetState())), "Uninitialized");

	// Initializing without provider returns InvalidArgument
	Status initNoProvider = manager.Initialize();
	CHECK(initNoProvider.IsError());
	CHECK_EQ(initNoProvider.GetCode(), ErrorCode::InvalidArgument);

	MemoryResourceProvider provider;
	manager.SetProvider(&provider);

	CHECK(manager.Initialize().IsOk());
	CHECK_EQ(manager.GetState(), ResourceManagerState::Ready);
	CHECK_EQ(std::string(ToString(manager.GetState())), "Ready");

	// Re-initializing returns InvalidState
	Status reinit = manager.Initialize();
	CHECK(reinit.IsError());
	CHECK_EQ(reinit.GetCode(), ErrorCode::InvalidState);

	// Shutdown
	CHECK(manager.Shutdown().IsOk());
	CHECK_EQ(manager.GetState(), ResourceManagerState::ShutdownState);
	CHECK_EQ(std::string(ToString(manager.GetState())), "Shutdown");

	// Operations after shutdown return NotAllowed
	Status postInit = manager.Initialize();
	CHECK_EQ(postInit.GetCode(), ErrorCode::NotAllowed);

	Status postShutdown = manager.Shutdown();
	CHECK_EQ(postShutdown.GetCode(), ErrorCode::NotAllowed);

	auto id = ResourceId::Create("test/res").GetValue();
	auto postLoad = manager.Load(id);
	CHECK_EQ(postLoad.GetError(), ErrorCode::InvalidState);
}

MODERN_TEST(ClientResources_InvalidOperationsReturnErrorsNotThrow)
{
	MemoryResourceProvider provider;
	ResourceManager manager;
	manager.SetProvider(&provider);

	// Load before Initialize returns InvalidState
	ResourceId defaultId;
	auto loadBeforeInit = manager.Load(defaultId);
	CHECK_EQ(loadBeforeInit.GetError(), ErrorCode::InvalidState);

	CHECK(manager.Initialize().IsOk());

	// Load with invalid default id returns InvalidArgument
	auto loadInvalid = manager.Load(defaultId);
	CHECK_EQ(loadInvalid.GetError(), ErrorCode::InvalidArgument);

	// Provider with invalid id
	CHECK_EQ(provider.RegisterResource(defaultId, ResourceData()).GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(provider.UnregisterResource(defaultId).GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(provider.Load(defaultId).GetError(), ErrorCode::InvalidArgument);
	CHECK(!provider.HasResource(defaultId));
}

// ---------------------------------------------------------------------------
// Main Test Runner
// ---------------------------------------------------------------------------

int main()
{
	std::printf("Modern CLIENT-005 resource boundary tests\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n", static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
		failedCases,
		static_cast<int>(ModernTests::Registry().size()),
		ModernTests::FailureCount());
	return 1;
}

