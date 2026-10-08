#pragma once

#include "character/Character.h"
#include "entity/Entity.h"
#include "equipment/EquipmentState.h"
#include "item/ItemDefinition.h"
#include "math/Vector3.h"
#include "types/Ids.h"
#include "types/Result.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <type_traits>
#include <vector>

// A deliberately small test harness.
//
// CORE-001 needs to assert behaviour, not to ship a test framework. A test
// binary that links Modern and nothing else is the whole point, and pulling
// in a dependency to do that would undercut it. Cases self-register at static
// initialisation time; RunAll() executes them in registration order and
// reports a failure count.

namespace ModernTests
{
	struct TestCase
	{
		const char* name;
		void (*run)();
	};

	inline std::vector<TestCase>& Registry()
	{
		static std::vector<TestCase> cases;
		return cases;
	}

	inline int& FailureCount()
	{
		static int count = 0;
		return count;
	}

	// Renders a value for failure output. The primary template prints
	// "<?>" so an undescribed type is visible as a gap in the harness rather
	// than as a failure to compile.
	template <typename T, typename Enable = void>
	struct Describer
	{
		static constexpr bool Known = false;
		static std::string Get(const T&) { return "<?>"; }
	};

	template <typename T>
	struct Describer<T, std::enable_if_t<std::is_integral<T>::value && !std::is_same<T, bool>::value>>
	{
		static constexpr bool Known = true;
		static std::string Get(const T& value) { return std::to_string(static_cast<long long>(value)); }
	};

	template <typename T>
	struct Describer<T, std::enable_if_t<std::is_floating_point<T>::value>>
	{
		static constexpr bool Known = true;
		static std::string Get(const T& value) { return std::to_string(static_cast<double>(value)); }
	};

	template <>
	struct Describer<bool>
	{
		static constexpr bool Known = true;
		static std::string Get(const bool& value) { return value ? "true" : "false"; }
	};

	template <>
	struct Describer<std::string>
	{
		static constexpr bool Known = true;
		static std::string Get(const std::string& value) { return "\"" + value + "\""; }
	};

	template <>
	struct Describer<const char*>
	{
		static constexpr bool Known = true;
		static std::string Get(const char* const& value) { return value ? std::string(value) : std::string("nullptr"); }
	};

	template <>
	struct Describer<Modern::ErrorCode>
	{
		static constexpr bool Known = true;
		static std::string Get(const Modern::ErrorCode& value) { return Modern::ToString(value); }
	};

	template <>
	struct Describer<Modern::EntityState>
	{
		static constexpr bool Known = true;
		static std::string Get(const Modern::EntityState& value) { return Modern::ToString(value); }
	};

	template <>
	struct Describer<Modern::CharacterClass>
	{
		static constexpr bool Known = true;
		static std::string Get(const Modern::CharacterClass& value) { return Modern::ToString(value); }
	};

template <>
struct Describer<Modern::ItemKind>
{
	static constexpr bool Known = true;
	static std::string Get(const Modern::ItemKind& value) { return Modern::ToString(value); }
};

template <>
struct Describer<Modern::Vector3>
{
	static constexpr bool Known = true;
	static std::string Get(const Modern::Vector3& value)
	{
		return "(" + std::to_string(static_cast<double>(value.x)) + ", " +
			std::to_string(static_cast<double>(value.y)) + ", " +
			std::to_string(static_cast<double>(value.z)) + ")";
	}
};

template <typename Tag, typename Underlying>
struct Describer<Modern::detail::StrongId<Tag, Underlying>>
	{
		static constexpr bool Known = true;
		static std::string Get(const Modern::detail::StrongId<Tag, Underlying>& value)
		{
			return std::to_string(static_cast<long long>(value.Get()));
		}
	};

	inline void CheckImpl(
		bool        condition,
		const char* expression,
		const char* file,
		int         line)
	{
		if (!condition)
		{
			++FailureCount();
			std::printf("    FAIL %s:%d  %s\n", file, line, expression);
		}
	}

	template <typename A, typename B>
	void CheckEqImpl(
		const A&    actual,
		const B&    expected,
		const char* expression,
		const char* file,
		int         line)
	{
		if (actual == expected)
		{
			return;
		}

		++FailureCount();
		std::printf("    FAIL %s:%d  %s\n", file, line, expression);
		std::printf("         actual:   %s\n", Describer<A>::Get(actual).c_str());
		std::printf("         expected: %s\n", Describer<B>::Get(expected).c_str());
	}

	struct Registrar
	{
		Registrar(const char* name, void (*run)())
		{
			Registry().push_back(TestCase{ name, run });
		}
	};

	inline int RunAll()
	{
		int failedCases = 0;

		for (const TestCase& test : Registry())
		{
			const int before = FailureCount();
			std::printf("  %s\n", test.name);

			test.run();

			if (FailureCount() != before)
			{
				++failedCases;
			}
		}

		return failedCases;
	}
}

#define MODERN_TEST(name)                                                  \
    static void name();                                                   \
    static ::ModernTests::Registrar modern_test_registrar_##name(#name, &name); \
    static void name()

#define CHECK(expr) ::ModernTests::CheckImpl((expr), #expr, __FILE__, __LINE__)

#define CHECK_EQ(actual, expected) \
    ::ModernTests::CheckEqImpl((actual), (expected), #actual " == " #expected, __FILE__, __LINE__)

#define CHECK_NE(actual, expected) \
    ::ModernTests::CheckImpl((actual) != (expected), #actual " != " #expected, __FILE__, __LINE__)

#define CHECK_GT(actual, expected) \
    ::ModernTests::CheckImpl((actual) > (expected), #actual " > " #expected, __FILE__, __LINE__)

#define CHECK_GE(actual, expected) \
    ::ModernTests::CheckImpl((actual) >= (expected), #actual " >= " #expected, __FILE__, __LINE__)

#define CHECK_LT(actual, expected) \
    ::ModernTests::CheckImpl((actual) < (expected), #actual " < " #expected, __FILE__, __LINE__)

#define CHECK_LE(actual, expected) \
    ::ModernTests::CheckImpl((actual) <= (expected), #actual " <= " #expected, __FILE__, __LINE__)

// REQUIRE is CHECK with a bail-out: on failure it records the failure AND
// returns from the test, so the rest of the case cannot run against a
// precondition that did not hold.
//
// It exists because a non-fatal CHECK is only safe when the code after it
// tolerates the failure. Dereferencing a std::optional is the case that does
// not: `CHECK(opt.has_value()); opt->GetCurrent(...)` reads the success
// value, flags a failure, and then dereferences nullopt anyway - which is an
// access violation, not a test failure, and takes the whole executable down
// with it rather than reporting the one broken assertion.
#define REQUIRE(expr)                                                          \
    do                                                                         \
    {                                                                          \
        if (!(expr))                                                           \
        {                                                                      \
            ::ModernTests::CheckImpl(false, #expr, __FILE__, __LINE__);        \
            return;                                                            \
        }                                                                      \
    } while (false)
