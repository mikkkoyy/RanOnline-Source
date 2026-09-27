#pragma once

#include <cstdint>
#include <optional>
#include <utility>

namespace Modern
{
	// Result conventions.
	//
	// The core reports failures by value, not by exception: game rules run in a
	// fixed-order simulation loop where an exception thrown from deep inside a
	// stat calculation is far harder to reason about than a returned code.
	// This is a deliberately small convention — an enum, a status wrapper and
	// an optional value — not an error-handling framework.

	enum class ErrorCode : uint8_t
	{
		None = 0,

		// A caller supplied an argument the callee cannot accept: an invalid
		// id, an empty name, a non-finite position, a negative amount.
		InvalidArgument,

		// A lookup found nothing. Only returned by operations that search.
		NotFound,

		// A create/insert operation found the identity already present.
		AlreadyExists,

		// The object is in a lifecycle state that forbids the operation, e.g.
		// spawning an already-spawned character.
		InvalidState,

		// The object is in a valid state but the operation is not permitted for
		// it at all, e.g. a transition out of a terminal state.
		NotAllowed,
	};

	// Human-readable name of an error code. Stable for logs and test output.
	const char* ToString(ErrorCode code) noexcept;

	constexpr bool IsOk(ErrorCode code) noexcept
	{
		return code == ErrorCode::None;
	}

	constexpr bool IsError(ErrorCode code) noexcept
	{
		return code != ErrorCode::None;
	}

	// A bare outcome: success or a single error code, carrying no value.
	class Status
	{
	public:
		Status() noexcept = default;
		explicit Status(ErrorCode code) noexcept : m_code(code) {}

		// Qualified: the unqualified name would resolve to these members
		// rather than the namespace-scope ErrorCode overloads.
		constexpr bool IsOk() const noexcept { return Modern::IsOk(m_code); }
		constexpr bool IsError() const noexcept { return Modern::IsError(m_code); }
		constexpr ErrorCode GetCode() const noexcept { return m_code; }

		// Not constexpr: ToString is defined out of line.
		const char* GetMessage() const noexcept { return ToString(m_code); }

		constexpr explicit operator bool() const noexcept { return IsOk(); }

		friend constexpr bool operator==(Status lhs, Status rhs) noexcept
		{
			return lhs.m_code == rhs.m_code;
		}

		friend constexpr bool operator!=(Status lhs, Status rhs) noexcept
		{
			return lhs.m_code != rhs.m_code;
		}

	private:
		ErrorCode m_code = ErrorCode::None;
	};

	// A bare success, as a named constant so intent is obvious at the call site.
	inline Status Ok() noexcept
	{
		return Status();
	}

	// A status carrying a value on success. On failure there is no value, and
	// reading one is a programming error rather than a recoverable condition.
	template <typename T>
	class Result
	{
	public:
		Result(Status status) : m_status(status) {}
		Result(T value) : m_value(std::move(value)) {}

		bool IsOk() const noexcept { return m_status.IsOk() && m_value.has_value(); }
		bool IsError() const noexcept { return !IsOk(); }
		ErrorCode GetError() const noexcept { return m_status.GetCode(); }
		const Status& GetStatus() const noexcept { return m_status; }
		const char* GetMessage() const noexcept { return m_status.GetMessage(); }

		explicit operator bool() const noexcept { return IsOk(); }

		const T& GetValue() const { return *m_value; }
		T& GetValue() { return *m_value; }

		const T* TryGetValue() const noexcept { return m_value.has_value() ? &*m_value : nullptr; }

		T GetValueOr(T fallback) const
		{
			return m_value.has_value() ? *m_value : fallback;
		}

	private:
		Status       m_status;
		std::optional<T> m_value;
	};
}
