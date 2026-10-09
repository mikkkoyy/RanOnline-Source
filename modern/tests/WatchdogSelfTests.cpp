// TEST STALL WATCHDOG - deterministic self-test.
//
// WHAT THIS IS FOR
//
// The watchdog in TestHarness.h exists because an intermittent test hang was
// observed once and, being an indefinite process, had to be killed from
// outside - which destroys the only evidence that mattered: which test was
// running. A guard that has never been seen to fire is not a guard; it is a
// hope. This proves the guard fires, names the right test, flushes what it
// printed, and exits with the code it promises.
//
// WHY IT IS A SEPARATE EXECUTABLE, AND NOT A CTEST TEST
//
// The thing under test terminates its own process. Run inside the ordinary
// suites it would take the whole suite down and make every normal CTest run
// look broken, so it is deliberately walled off in its own binary and is NOT
// registered with add_test. It is run explicitly and judges itself by exit code.
//
// WHAT IT ASSERTS
//
//   1. a test that blocks forever is reported rather than waited on;
//   2. the report names the CORRECT test;
//   3. the report was flushed before the abrupt exit;
//   4. the process exits with kWatchdogExitCode (3);
//   5. a prompt test is NOT reported - a guard that fires on everything detects
//      nothing, so false positives are tested as seriously as false negatives.
//
// The budget in this binary is deliberately short so the self-test finishes in
// seconds. The production budget in TestHarness.h is 45s and is not changed by
// anything here. No sleep is added to any production test.

#include "TestHarness.h"

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>

// CreateProcess rather than std::system.
//
// std::system runs the command through cmd.exe, and MSVC wraps it as
// `cmd /c "<command>"`. When the command itself begins with a quote - which it
// must, because this repository's path CONTAINS A SPACE - cmd's quote-parsing
// rule strips the outer pair and then splits the remainder on the space. The
// result was `'D:\FILES\project\modernization' is not recognized`, which looks
// exactly like the watchdog failing to do its job. Going straight to
// CreateProcess removes the shell from the path entirely, and gives the exact
// child exit code this test exists to assert on.
#include <windows.h>

namespace
{
	// The budget for this binary only - parent AND child. Short, so the whole
	// self-test is a few seconds rather than 45.
	constexpr long long kSelfTestBudgetMs = 900;

	constexpr char kStallingTestName[] = "WatchdogSelfTest_ThisOneBlocksForever";
	constexpr char kFastTestName[]      = "WatchdogSelfTest_ThisOneReturnsPromptly";
	constexpr char kReportMarker[]      = "TEST STALL TIMEOUT";

	// Blocks forever. The point of the exercise: a thread that can never return,
	// joined by nobody. The watchdog must not wait for it.
	void BlockForever()
	{
		for (;;)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}

	// Returns immediately, to prove the guard does not fire on ordinary tests.
	void ReturnPromptly() {}

	// Owns the child's output file handle, so it is closed however this function
	// exits.
	struct OutputFile
	{
		HANDLE handle = INVALID_HANDLE_VALUE;

		explicit OutputFile(const char* path)
		{
			// FILE_SHARE_READ is REQUIRED: the parent re-opens this same file to
			// read the child's output back, and a write handle that forbids
			// readers makes that open fail with a sharing violation. The symptom
			// is an empty string, which is indistinguishable from the child having
			// printed nothing - which is exactly the wrong conclusion to draw when
			// the watchdog is under test.
			// The security attributes are REQUIRED, not decoration. A handle
			// created with a null SECURITY_ATTRIBUTES is NOT inheritable, so
			// bInheritHandles = TRUE on its own still leaves the child with no
			// usable stdout - and the watchdog's report, which is the entire
			// thing under test, goes nowhere. bInheritHandle = TRUE is what makes
			// STARTF_USESTDHANDLES actually route output to our file.
			SECURITY_ATTRIBUTES attributes = {};
			attributes.nLength        = sizeof(attributes);
			attributes.bInheritHandle = TRUE;

			handle = CreateFileA(path, GENERIC_WRITE,
			                     FILE_SHARE_READ | FILE_SHARE_WRITE, &attributes,
			                     CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		}

		// Flushes and closes, so the read-back sees every byte the child wrote.
		void Close() noexcept
		{
			if (handle != INVALID_HANDLE_VALUE)
			{
				CloseHandle(handle);
				handle = INVALID_HANDLE_VALUE;
			}
		}

		~OutputFile()
		{
			if (handle != INVALID_HANDLE_VALUE)
			{
				CloseHandle(handle);
			}
		}

		OutputFile(const OutputFile&) = delete;
		OutputFile& operator=(const OutputFile&) = delete;
	};

	// Runs the child with stdout and stderr merged into `outPath`, and returns
	// its merged output. `exitCodeOut` receives the child's real exit code.
	std::string RunChild(const std::string& exe, const char* mode, const char* testName,
	                     const char* outPath, int& exitCodeOut)
	{
		exitCodeOut = -1;
		OutputFile  out(outPath);

		if (out.handle == INVALID_HANDLE_VALUE)
		{
			return std::string();
		}

		// lpApplicationName is the real path, so the executable is never resolved
		// by cmd or by a search path. lpCommandLine only has to carry argv[0] and
		// the two arguments, quoted conventionally.
		std::string commandLine = "\"" + exe + "\" ";
		commandLine += mode;
		commandLine += " \"";
		commandLine += testName;
		commandLine += "\"";

		std::vector<char> mutableCommand(commandLine.begin(), commandLine.end());
		mutableCommand.push_back('\0');

		STARTUPINFOA startup = {};
		startup.cb      = sizeof(startup);
		startup.dwFlags = STARTF_USESTDHANDLES;
		// The child inherits nothing, so stdin is a real NUL device rather than
		// our console: a child that reads stdin cannot steal the runner's.
		// (The guard never reads stdin; this is belt and braces.)
		startup.hStdInput  = nullptr;
		startup.hStdOutput = out.handle;
		startup.hStdError  = out.handle;

		PROCESS_INFORMATION process = {};

		// bInheritHandles MUST be TRUE here. STARTF_USESTDHANDLES only routes the
		// child's stdout/stderr through the handles named above if those handles are
		// actually inherited. With FALSE the child receives none of them and every
		// byte the watchdog prints - including the stall report this whole test
		// exists to check - goes nowhere. That is precisely the failure mode this
		// self-test was written to catch, and it cost a build cycle to find.
		if (CreateProcessA(exe.c_str(), mutableCommand.data(), nullptr, nullptr, TRUE,
		                   CREATE_NO_WINDOW, nullptr, nullptr, &startup,
		                   &process) == FALSE)
		{
			std::printf("  [self-test] CreateProcess failed for %s (win32 %lu)\n",
			            exe.c_str(), static_cast<unsigned long>(GetLastError()));
			return std::string();
		}

		WaitForSingleObject(process.hProcess, INFINITE);

		DWORD code = 0;
		if (GetExitCodeProcess(process.hProcess, &code) != FALSE)
		{
			exitCodeOut = static_cast<int>(code);
		}

		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);

		// The child is gone and has flushed, so the write end is closed before the
		// read-back.
		out.Close();

		std::string output;

		HANDLE read = CreateFileA(outPath, GENERIC_READ, FILE_SHARE_READ, nullptr,
		                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (read != INVALID_HANDLE_VALUE)
		{
			char   buffer[512];
			DWORD  bytesRead = 0;
			while (ReadFile(read, buffer, static_cast<DWORD>(sizeof(buffer) - 1),
			                &bytesRead, nullptr) != FALSE &&
			       bytesRead > 0)
			{
				buffer[bytesRead] = '\0';
				output += buffer;
			}
			CloseHandle(read);
		}

		std::remove(outPath);
		return output;
	}
} // namespace

// ---------------------------------------------------------------------------
// CHILD MODES - register exactly ONE test and run it through the shared harness,
// so the child exercises the real watchdog path rather than a copy of it.
// ---------------------------------------------------------------------------
static int RunChildStalling(const char* name)
{
	// Set BEFORE RunAll: the budget is captured when the watchdog thread starts,
	// which happens inside RunAll.
	::ModernTests::g_watchdogBudgetMs = kSelfTestBudgetMs;

	::ModernTests::Registrar registrar(name, &BlockForever);
	(void) registrar;

	const int failed = ::ModernTests::RunAll();
	std::printf("[child] completed WITHOUT stalling, failed=%d\n", failed);
	std::fflush(stdout);
	return failed == 0 ? 0 : 1;
}

static int RunChildFast(const char* name)
{
	::ModernTests::g_watchdogBudgetMs = kSelfTestBudgetMs;

	::ModernTests::Registrar registrar(name, &ReturnPromptly);
	(void) registrar;

	const int failed = ::ModernTests::RunAll();
	std::printf("[child] completed, failed=%d\n", failed);
	std::fflush(stdout);
	return failed == 0 ? 0 : 1;
}

// ---------------------------------------------------------------------------
// PARENT MODE - the assertions.
// ---------------------------------------------------------------------------

// The stalling child must be killed by the watchdog: report present, correct
// test named, limit printed, exit code exactly kWatchdogExitCode.
static int VerifyStallIsReported(const std::string& exe)
{
	int exitCode = -1;

	const std::string output =
	    RunChild(exe, "run-stalling", kStallingTestName, "watchdog_selftest_stall.txt",
	             exitCode);

	std::printf("---- stalling child output ----\n%s------------------------------\n",
	            output.c_str());

	const bool hasMarker = output.find(kReportMarker) != std::string::npos;
	const bool namedTest = output.find(kStallingTestName) != std::string::npos;
	const bool hasLimit  = output.find("limit:") != std::string::npos;
	const bool rightCode = exitCode == ::ModernTests::kWatchdogExitCode;

	std::printf("child exit code   : %d (expected %d)\n", exitCode,
	            ::ModernTests::kWatchdogExitCode);
	std::printf("report present    : %s\n", hasMarker ? "yes" : "NO");
	std::printf("correct test named: %s\n", namedTest ? "yes" : "NO");
	std::printf("limit printed     : %s\n", hasLimit ? "yes" : "NO");

	if (!hasMarker || !namedTest || !hasLimit || !rightCode)
	{
		std::printf("SELF-TEST FAILED: watchdog did not behave as specified\n");
		std::printf("  (empty child output means the child never ran - a spawn\n");
		std::printf("   problem, not a watchdog failure)\n");
		return 1;
	}

	std::printf("SELF-TEST OK: stall reported, named, flushed, exit code %d\n",
	            exitCode);
	return 0;
}

// A prompt test must complete normally, with no report and exit code 0.
static int VerifyFastTestIsNotReported(const std::string& exe)
{
	int exitCode = -1;

	const std::string output =
	    RunChild(exe, "run-fast", kFastTestName, "watchdog_selftest_fast.txt", exitCode);

	const bool reported = output.find(kReportMarker) != std::string::npos;

	std::printf("fast child exit code : %d (expected 0)\n", exitCode);
	std::printf("reported as stall   : %s\n", reported ? "YES - WRONG" : "no");

	if (reported || exitCode != 0)
	{
		std::printf("SELF-TEST FAILED: a prompt test must not trip the watchdog\n");
		return 1;
	}

	std::printf("SELF-TEST OK: prompt test neither stalled nor misreported\n");
	return 0;
}

int main(int argc, char** argv)
{
	std::setvbuf(stdout, nullptr, _IONBF, 0);

	if (argc >= 3)
	{
		const std::string mode = argv[1];
		if (mode == "run-stalling")
		{
			return RunChildStalling(argv[2]);
		}
		if (mode == "run-fast")
		{
			return RunChildFast(argv[2]);
		}
	}

	// Parent mode. Children are launched by absolute path so they do not depend
	// on the working directory.
	//
	// argv[0] is not used verbatim: a launcher that quotes the program path
	// (which this repo's space-containing path forces) hands argv[0] over with
	// the quotes still attached.
	std::string exe = (argc >= 1) ? argv[0] : "ModernWatchdogSelfTests.exe";
	while (!exe.empty() && exe.front() == '"')
	{
		exe.erase(exe.begin());
	}
	while (!exe.empty() && exe.back() == '"')
	{
		exe.pop_back();
	}

	int failures = 0;

	std::printf("Watchdog self-test (budget %lld ms; production default is %lld ms)\n\n",
	            kSelfTestBudgetMs, ::ModernTests::kWatchdogBudgetMilliseconds);

	std::printf("== scenario 1: a test that blocks forever ==\n");
	failures += VerifyStallIsReported(exe);

	std::printf("\n== scenario 2: a test that returns promptly ==\n");
	failures += VerifyFastTestIsNotReported(exe);

	std::printf("\n");
	if (failures == 0)
	{
		std::printf("ALL WATCHDOG SELF-TESTS PASSED\n");
		return 0;
	}

	std::printf("%d WATCHDOG SELF-TEST(S) FAILED\n", failures);
	return 1;
}