#pragma once

#include <cstdint>

namespace Modern
{
	// Codex stat increases from character progression (legacy m_dw*Increase fields)
	// All values are DWORD/unsigned in legacy - use uint32_t for exact representation
	struct CodexContribution
	{
		// Resource increases (added AFTER rate multiplication in legacy)
		uint32_t hpIncrease = 0;
		uint32_t mpIncrease = 0;
		uint32_t spIncrease = 0;

		// Combat stat increases
		uint32_t hitRateIncrease = 0;
		uint32_t avoidRateIncrease = 0;
		uint32_t defenseIncrease = 0;
		uint32_t attackIncrease = 0;

		// Resistance increase (applied to all resist elements)
		uint32_t resistanceIncrease = 0;

		void Reset()
		{
			*this = {};
		}
	};

	// Provider interface for codex contributions
	class ICodexProvider
	{
	public:
		virtual ~ICodexProvider() = default;

		virtual CodexContribution GetCodexContribution() const = 0;
	};

	// Test provider with deterministic values
	class TestCodexProvider : public ICodexProvider
	{
	public:
		TestCodexProvider() = default;

		CodexContribution GetCodexContribution() const override
		{
			CodexContribution c;
			c.hpIncrease = 100;
			c.mpIncrease = 50;
			c.spIncrease = 30;
			c.hitRateIncrease = 10;
			c.avoidRateIncrease = 5;
			c.defenseIncrease = 20;
			c.attackIncrease = 15;
			c.resistanceIncrease = 10;
			return c;
		}
	};
}