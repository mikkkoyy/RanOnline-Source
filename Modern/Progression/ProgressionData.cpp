#include "ProgressionData.h"

#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace Modern
{
	bool RANProgressionData::LoadFromTextFile(const char* filePath)
	{
		std::ifstream f(filePath);
		if (!f) return false;

		std::string line;
		m_table.clear();
		m_table.reserve(300);

		while (std::getline(f, line))
		{
			if (line.empty()) continue;
			if (line[0] == '#' || line[0] == ';') continue;

			size_t start = line.find_first_not_of(" \t\r");
			if (start == std::string::npos) continue;

			char* end = nullptr;
			const char* cstr = line.c_str() + start;
			int64_t val = std::strtoll(cstr, &end, 10);
			if (end != cstr)
				m_table.push_back(val);
		}

		return !m_table.empty();
	}

	bool RANProgressionData::LoadFromText(const char* text, size_t length)
	{
		m_table.clear();
		m_table.reserve(300);

		std::string_view sv(text, length);
		size_t pos = 0;
		while (pos < length)
		{
			size_t eol = sv.find('\n', pos);
			std::string_view line = sv.substr(pos, (eol == std::string_view::npos ? length : eol) - pos);
			pos = (eol == std::string_view::npos) ? length : eol + 1;

			if (line.empty()) continue;
			if (line[0] == '#' || line[0] == ';') continue;

			size_t start = line.find_first_not_of(" \t\r");
			if (start == std::string_view::npos) continue;

			char* end = nullptr;
			const char* cstr = line.data() + start;
			int64_t val = std::strtoll(cstr, &end, 10);
			if (end != cstr)
				m_table.push_back(val);
		}

		return !m_table.empty();
	}
}