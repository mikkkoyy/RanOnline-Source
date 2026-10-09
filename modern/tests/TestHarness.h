#pragma once

#include "character/Character.h"
#include "entity/Entity.h"
#include "equipment/EquipmentState.h"
#include "item/ItemDefinition.h"
#include "math/Vector3.h"
#include "types/Ids.h"
#include "types/Result.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
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

	// -------------------------------------------------------------------------
	// TEST STALL WATCHDOG
	//
	// A test that blocks forever is an indefinite process, not a test result. It
	// has to be killed from outside, which destroys the only evidence that
	// mattered: WHICH test was running when it stopped making progress. This
	// watchdog converts that into a bounded, self-reporting failure.
	//
	// It reports, and it never masks:
	//
	//   * it exits with kWatchdogExitCode, which is neither 0 (pass) nor 1
	//     (assertion failure), so a stall can never be read as either;
	//   * it exits without joining the blocked thread and without running
	//     destructors, so nothing that is stuck can delay the report;
	//   * a normal assertion failure does NOT trip it - the budget is about
	//     elapsed time, and a failing assertion still returns promptly.
	//
	// The clock is monotonic, and the deadline is set once per test when the test
	// starts. Nothing the test does can push it out - not logging, not unrelated
	// background activity, not a tight loop.
	//
	// It is deliberately NOT a general monitoring framework. It does not inspect
	// or mutate fixtures, take references to test state, or call back into
	// anything a test owns. The only thing it reads is the current test NAME,
	// which points at static storage (the registration table) and so outlives
	// every test and every fixture.
	// -------------------------------------------------------------------------

	// The exit code a stalled run terminates with. Distinct from 0 and 1 on
	// purpose: 3 is "the runner could not get an answer", which is a third
	// thing and must stay distinguishable from both real outcomes.
	inline constexpr int kWatchdogExitCode = 3;

	// Longest a single test may take. Comfortably above the slowest legitimate
	// case and far below an indefinite stall. Nothing in the suites waits
	// anywhere near this: the longest deliberate wait is a few seconds.
	inline constexpr long long kWatchdogBudgetMilliseconds = 45000;

	// The name of the test currently executing, or nullptr when none is.
	//
	// Points into the registration table, which is static storage built at
	// static-initialisation time and never mutated again, so the watchdog thread
	// can read it without any risk of a dangling pointer or a torn value.
	inline std::atomic<const char*>& CurrentTestName() noexcept
	{
		static std::atomic<const char*> name{nullptr};
		return name;
	}

	// Watchdog state. Started once by RunAll, joined once on the way out.
	inline std::thread& WatchdogThread() noexcept
	{
		static std::thread thread;
		return thread;
	}

	inline std::atomic<bool>& WatchdogStop() noexcept
	{
		static std::atomic<bool> stop{false};
		return stop;
	}

	// The budget is read by the watchdog thread, so it is a plain value written
	// once in StartWatchdog before the thread is created. Publishing it through
	// the thread creation itself is the synchronisation - std::thread's
	// constructor happens-before everything the new thread does.
	inline long long g_watchdogBudgetMs = kWatchdogBudgetMilliseconds;

	// When the currently-armed test started, on the same monotonic clock.
	// Written by TestWatchdogScope on the main thread before each test and read
	// by the watchdog. Zero means "no test armed".
	inline std::atomic<long long>& CurrentTestStartMs() noexcept
	{
		static std::atomic<long long> value{0};
		return value;
	}

	inline void WatchdogLoop() noexcept
	{
		// How often the deadline is re-checked. This is a polling interval, not
		// the budget: the budget below is what decides a stall, so a coarse
		// check only affects how promptly the report appears.
		constexpr auto kPollInterval = std::chrono::milliseconds(250);

		const long long budget = g_watchdogBudgetMs;

		for (;;)
		{
			std::this_thread::sleep_for(kPollInterval);

			if (WatchdogStop().load(std::memory_order_acquire))
			{
				return;
			}

			const char* name = CurrentTestName().load(std::memory_order_acquire);
			if (name == nullptr)
			{
				// Between tests, or after the last one. Not a stall: nothing is
				// supposed to be running.
				continue;
			}

			// Monotonic, and per-test: the deadline is computed from when THIS
			// test started, so it cannot be extended by anything the test does.
			const long long start =
			    CurrentTestStartMs().load(std::memory_order_acquire);
			if (start == 0)
			{
				continue;
			}

			const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			                         std::chrono::steady_clock::now().time_since_epoch())
			                         .count() -
			                     start;

			if (elapsed < budget)
			{
				continue;
			}

			std::printf(
			    "\n"
			    "================================================================\n"
			    "TEST STALL TIMEOUT\n"
			    "  test : %s\n"
			    "  limit: %lld s\n"
			    "  stalled for: %.1f s\n"
			    "================================================================\n"
			    "The test exceeded its per-test time budget and has been\n"
			    "terminated. The exit code below is %d, which is neither\n"
			    "success (0) nor an assertion failure (1), so this stall cannot\n"
			    "be mistaken for a pass.\n"
			    "The process was ended deliberately: the blocked test was NOT\n"
			    "joined and no destructors ran, so a thread that cannot finish\n"
			    "cannot delay this report.\n",
			    name, budget / 1000,
			    static_cast<double>(elapsed) / 1000.0, kWatchdogExitCode);

			// Flush explicitly. The report is the entire point, and a buffered
			// stream would lose it at the abrupt exit below.
			std::fflush(stdout);
			std::fflush(stderr);

			std::_Exit(kWatchdogExitCode);
		}
	}

	// Arms the deadline for the test about to run, and disarms it when that test
	// returns - including when it returns early through a REQUIRE, a failed
	// assertion, or an exception.
	//
	// The scope is deliberately narrow: it covers the call to `body`, so the
	// deadline is measured over the test's own execution and not over the
	// harness bookkeeping around it.
	class TestWatchdogScope
	{
	public:
		explicit TestWatchdogScope(const char* name) noexcept
		{
			// Name first, then the clock: the watchdog treats a non-null name
			// with a zero start as "not armed yet" and keeps waiting, so the two
			// stores cannot make it report a spurious stall in between.
			CurrentTestStartMs().store(0, std::memory_order_release);
			CurrentTestName().store(name, std::memory_order_release);
			CurrentTestStartMs().store(NowMs(), std::memory_order_release);
		}

		TestWatchdogScope(const TestWatchdogScope&) = delete;
		TestWatchdogScope& operator=(const TestWatchdogScope&) = delete;

		~TestWatchdogScope()
		{
			// Cleared in this order so the watchdog never sees a live name with
			// no timestamp and cannot attribute a previous test's elapsed time
			// to the gap between two tests.
			CurrentTestName().store(nullptr, std::memory_order_release);
			CurrentTestStartMs().store(0, std::memory_order_release);
		}

	private:
		static long long NowMs() noexcept
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(
			           std::chrono::steady_clock::now().time_since_epoch())
			    .count();
		}
	};

	// Starts the watchdog thread. Called once by RunAll.
	inline void StartWatchdog() noexcept
	{
		WatchdogStop().store(false, std::memory_order_release);
		WatchdogThread() = std::thread(WatchdogLoop);
	}

	// Stops and JOINS the watchdog. Never detached: a detached watchdog would
	// outlive the static state it reads, and a joined one cannot.
	inline void StopWatchdog() noexcept
	{
		WatchdogStop().store(true, std::memory_order_release);
		if (WatchdogThread().joinable())
		{
			WatchdogThread().join();
		}
	}

	inline int RunAll()
	{
		StartWatchdog();

		int failedCases = 0;

		for (const TestCase& test : Registry())
		{
			const int before = FailureCount();
			std::printf("  %s\n", test.name);

			{
				// The guard's destructor clears the name on every exit path out
				// of the call below, so a test that returns by REQUIRE, by a
				// failing CHECK, or by an exception cannot leave its name armed
				// for the next one.
				TestWatchdogScope guard(test.name);
				test.run();
			}

			if (FailureCount() != before)
			{
				++failedCases;
			}
		}

		StopWatchdog();
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
