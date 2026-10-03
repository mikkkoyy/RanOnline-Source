#include "WinSockRuntime.h"

#include <winsock2.h>

#include <mutex>

namespace Modern::Network::WinSock
{
	namespace
	{
		// One process-wide counter, one lock.
		//
		// A function-local static rather than a namespace-scope one: it is
		// initialised on first use, which means a translation unit that reaches
		// this file cannot observe it half-built. The lock is what makes the
		// read-modify-write of the count atomic between threads; Winsock itself
		// is thread safe, its INITIALISATION and SHUTDOWN are the parts that are
		// not, and both happen under this lock.
		struct Runtime
		{
			std::mutex    mutex;
			std::size_t   references = 0;
		};

		Runtime& Instance() noexcept
		{
			static Runtime runtime;
			return runtime;
		}
	}

	std::size_t ReferenceCount() noexcept
	{
		Runtime& runtime = Instance();
		const std::lock_guard<std::mutex> lock(runtime.mutex);
		return runtime.references;
	}

	bool IsReady() noexcept
	{
		return ReferenceCount() != 0;
	}

	Status Acquire() noexcept
	{
		Runtime& runtime = Instance();
		const std::lock_guard<std::mutex> lock(runtime.mutex);

		// Only the 0 -> 1 transition may call WSAStartup. Calling it again while
		// already up is not merely wasteful: the startup info must match, and a
		// mismatched second call fails on some versions and is meaningless on
		// others.
		if (runtime.references == 0)
		{
			WSADATA data{};

			// 2.2, not 1.1. The public backread of legacy RAN confirms the client
			// and server are Winsock 2 with overlapped I/O and IOCP
			// (WSASocket(..., WSA_FLAG_OVERLAPPED) feeding CompletionPort work
			// items), so asking for 1.1 would be asking for less than RAN used.
			// The high byte is the version, the low byte the sub-version.
			const int result = ::WSAStartup(MAKEWORD(2, 2), &data);

			// Checked explicitly rather than via a macro, because a failed
			// startup must leave the counter at zero - otherwise the next caller
			// would believe Winsock is up when it never was, and every socket
			// call would fail with a confusing error instead of the real one.
			if (result != 0)
			{
				return Status(ErrorCode::InvalidState);
			}
		}

		++runtime.references;
		return Ok();
	}

	void Release() noexcept
	{
		Runtime& runtime = Instance();
		const std::lock_guard<std::mutex> lock(runtime.mutex);

		// Underflow is ignored on purpose. Reaching it means a caller released
		// more than it acquired, and the least harmful response is to keep
		// Winsock up: tearing it down on a bad count would break every OTHER
		// socket in the process, which is a far worse failure than the mistake
		// that caused it, and it would do so silently.
		if (runtime.references == 0)
		{
			return;
		}

		--runtime.references;
		if (runtime.references == 0)
		{
			// The last owner is leaving, so this is the point the brief calls the
			// shutdown of the owning network subsystem. Every Winsock call has
			// already stopped by the time a reference is dropped, because a
			// socket holds a reference for its whole lifetime.
			::WSACleanup();
		}
	}
}
