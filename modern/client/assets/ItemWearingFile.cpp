// ASURA-CHARACTER-001B: the bounded item-wearing-file reader.
//
// See ItemWearingFile.h for the contract. The implementation notes that are
// worth having next to the code rather than only in the header:
//
//   - The file is streamed one physical line at a time. `item.csv` has no
//     quoted field containing a newline, so a physical line is a record;
//     this is asserted rather than assumed, because a quoted field that ran
//     to end-of-line would otherwise desynchronise every row after it. A
//     line with an unterminated quote is counted as malformed and the
//     parser resynchronises on the next newline.
//
//   - Only the requested ids' rows are materialised. The other ~200k rows
//     are split (the split has to happen to find the id boundary) and then
//     discarded immediately.

#include "assets/ItemWearingFile.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace Modern::Client
{

namespace
{
    // Parses one RAN decimal id column. Returns false on anything that is not
    // exactly an unsigned decimal, so `50x`, ` 50` (untrimmed) or an empty
    // field never compares equal to a requested id.
    bool ParseUnsigned(const std::string& text, uint32_t& out)
    {
        if (text.empty()) { return false; }
        uint64_t value = 0;
        for (const char c : text)
        {
            if (c < '0' || c > '9') { return false; }
            value = value * 10u + static_cast<uint64_t>(c - '0');
            if (value > 0xFFFFFFFFull) { return false; }
        }
        out = static_cast<uint32_t>(value);
        return true;
    }

    void TrimInPlace(std::string& text)
    {
        const auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
        size_t      begin  = 0;
        while (begin < text.size() && isSpace(static_cast<unsigned char>(text[begin])))
        {
            ++begin;
        }
        size_t end = text.size();
        while (end > begin && isSpace(static_cast<unsigned char>(text[end - 1])))
        {
            --end;
        }
        text = text.substr(begin, end - begin);
    }

    // Reads one line, dropping a trailing CR so a CRLF file parses the same
    // as a LF one. Returns false at end of input.
    bool ReadLine(std::istream& stream, std::string& line)
    {
        if (!std::getline(stream, line)) { return false; }
        if (!line.empty() && line.back() == '\r') { line.pop_back(); }
        return true;
    }

    // "strWearingFileRight 3" -> 3. Returns false for any other spelling, so a
    // column named e.g. "strWearingFileRight" (no index) or
    // "strWearingFileRightX" is not silently treated as slot 0.
    bool TrailingSlotIndex(const std::string& name, const std::string& prefix, size_t& index)
    {
        if (name.size() <= prefix.size()) { return false; }
        if (name.compare(0, prefix.size(), prefix) != 0) { return false; }
        const std::string suffix = name.substr(prefix.size());
        for (const char c : suffix)
        {
            if (c < '0' || c > '9') { return false; }
        }
        const long value = std::strtol(suffix.c_str(), nullptr, 10);
        if (value < 0) { return false; }
        index = static_cast<size_t>(value);
        return true;
    }
} // namespace

Result<std::vector<std::string>> SplitCsvRecord(const std::string& record)
{
    std::vector<std::string> fields;
    std::string              field;
    bool                     inQuotes = false;
    size_t                   i        = 0;

    while (i < record.size())
    {
        const char c = record[i];
        if (inQuotes)
        {
            if (c == '"')
            {
                // RFC 4180 escape: "" inside a quoted field is a literal quote.
                if (i + 1 < record.size() && record[i + 1] == '"')
                {
                    field.push_back('"');
                    i += 2;
                    continue;
                }
                inQuotes = false;
                ++i;
                continue;
            }
            field.push_back(c);
            ++i;
            continue;
        }

        if (c == '"')
        {
            inQuotes = true;
            ++i;
            continue;
        }
        if (c == ',')
        {
            fields.push_back(field);
            field.clear();
            ++i;
            continue;
        }
        field.push_back(c);
        ++i;
    }

    if (inQuotes)
    {
        return Status(ErrorCode::InvalidArgument);
    }

    fields.push_back(field);
    return Result<std::vector<std::string>>(std::move(fields));
}

Status ItemWearingFileReader::Initialize(const std::filesystem::path& itemCsvPath)
{
    m_path = itemCsvPath;
    m_initialized = false;

    if (itemCsvPath.empty()) { return Status(ErrorCode::InvalidArgument); }

    std::error_code ec;
    if (!std::filesystem::is_regular_file(itemCsvPath, ec) || ec)
    {
        return Status(ErrorCode::NotFound);
    }

    std::ifstream stream(itemCsvPath, std::ios::binary);
    if (!stream) { return Status(ErrorCode::NotFound); }

    std::string headerLine;
    if (!ReadLine(stream, headerLine)) { return Status(ErrorCode::InvalidArgument); }

    const Result<std::vector<std::string>> headerFields = SplitCsvRecord(headerLine);
    if (headerFields.IsError()) { return Status(ErrorCode::InvalidArgument); }
    const std::vector<std::string>& header = headerFields.GetValue();

    // Locate every column we need BY NAME. Nothing here assumes a position.
    bool haveMain = false;
    bool haveSub  = false;
    m_rightColumns.assign(kWearingFileSlots, static_cast<size_t>(-1));
    m_leftColumns.assign(kWearingFileSlots, static_cast<size_t>(-1));

    for (size_t column = 0; column < header.size(); ++column)
    {
        const std::string& name = header[column];
        if (name == ItemCsvColumns::kMainId) { m_mainIdColumn = column; haveMain = true; continue; }
        if (name == ItemCsvColumns::kSubId)  { m_subIdColumn  = column; haveSub  = true; continue; }

        size_t slot = 0;
        if (TrailingSlotIndex(name, ItemCsvColumns::kRightPrefix, slot))
        {
            if (slot < kWearingFileSlots) { m_rightColumns[slot] = column; }
            continue;
        }
        if (TrailingSlotIndex(name, ItemCsvColumns::kLeftPrefix, slot))
        {
            if (slot < kWearingFileSlots) { m_leftColumns[slot] = column; }
            continue;
        }
    }

    if (!haveMain || !haveSub)
    {
        return Status(ErrorCode::InvalidArgument);
    }

    // At least one wearing-file column must exist, or the file is not the
    // table this reader is for. Refuse rather than return empty successes.
    const bool anyRight = std::any_of(m_rightColumns.begin(), m_rightColumns.end(),
                                      [](size_t c) { return c != static_cast<size_t>(-1); });
    const bool anyLeft = std::any_of(m_leftColumns.begin(), m_leftColumns.end(),
                                     [](size_t c) { return c != static_cast<size_t>(-1); });
    if (!anyRight && !anyLeft)
    {
        return Status(ErrorCode::InvalidArgument);
    }

    m_initialized = true;
    return Status();
}

Result<std::vector<ItemWearingFiles>> ItemWearingFileReader::Extract(
    const std::vector<NativeId>& requested, ItemWearingFileReport& report)
{
    report = ItemWearingFileReport();

    if (!m_initialized) { return Status(ErrorCode::InvalidState); }
    if (requested.empty()) { return Status(ErrorCode::InvalidArgument); }

    // Build the wanted set, and refuse a duplicated request rather than
    // reporting the same id twice as "missing".
    std::unordered_map<uint64_t, size_t> wantedIndex;
    for (size_t i = 0; i < requested.size(); ++i)
    {
        const uint64_t key = (static_cast<uint64_t>(requested[i].mainId) << 32) | requested[i].subId;
        if (!wantedIndex.emplace(key, i).second)
        {
            return Status(ErrorCode::InvalidArgument);
        }
    }

    std::vector<ItemWearingFiles> results(requested.size());
    std::vector<size_t>           hitCount(requested.size(), 0);
    std::vector<bool>             found(requested.size(), false);
    for (size_t i = 0; i < requested.size(); ++i) { results[i].id = requested[i]; }

    std::ifstream stream(m_path, std::ios::binary);
    if (!stream) { return Status(ErrorCode::NotFound); }

    std::string line;
    if (!ReadLine(stream, line))
    {
        return Status(ErrorCode::InvalidArgument);
    }

    const size_t requiredColumns = m_subIdColumn + 1 > m_mainIdColumn + 1
                                       ? m_subIdColumn + 1
                                       : m_mainIdColumn + 1;

    while (ReadLine(stream, line))
    {
        if (line.empty()) { continue; }
        ++report.rowsScanned;

        const Result<std::vector<std::string>> split = SplitCsvRecord(line);
        if (split.IsError())
        {
            ++report.rowsMalformed;
            continue;
        }
        const std::vector<std::string>& fields = split.GetValue();

        // A short row cannot hold the id columns, let alone the wearing-file
        // columns: malformed, not a match.
        if (fields.size() < requiredColumns)
        {
            ++report.rowsMalformed;
            continue;
        }

        uint32_t mainId = 0;
        uint32_t subId  = 0;
        if (!ParseUnsigned(fields[m_mainIdColumn], mainId) ||
            !ParseUnsigned(fields[m_subIdColumn], subId))
        {
            // RAN's first data rows are padding (the header is followed by
            // blank/notes lines in some exports). An unparseable id is not a
            // match; it is only a malformed row if it was otherwise short.
            ++report.rowsMalformed;
            continue;
        }

        const uint64_t key = (static_cast<uint64_t>(mainId) << 32) | subId;
        const auto     it  = wantedIndex.find(key);
        if (it == wantedIndex.end()) { continue; }

        const size_t index = it->second;
        ++hitCount[index];
        found[index] = true;

        ItemWearingFiles& row = results[index];
        for (size_t slot = 0; slot < kWearingFileSlots; ++slot)
        {
            if (m_rightColumns[slot] != static_cast<size_t>(-1) &&
                m_rightColumns[slot] < fields.size())
            {
                row.right[slot] = fields[m_rightColumns[slot]];
                TrimInPlace(row.right[slot]);
            }
            if (m_leftColumns[slot] != static_cast<size_t>(-1) &&
                m_leftColumns[slot] < fields.size())
            {
                row.left[slot] = fields[m_leftColumns[slot]];
                TrimInPlace(row.left[slot]);
            }
        }
        ++report.rowsMatched;
    }

    std::vector<size_t> keepOrder;
    for (size_t i = 0; i < requested.size(); ++i)
    {
        if (found[i])
        {
            if (hitCount[i] > size_t{1}) { ++report.duplicateIds; }
            keepOrder.push_back(i);
        }
        else
        {
            report.missingIds.push_back(requested[i]);
        }
    }

    if (keepOrder.empty())
    {
        return Status(ErrorCode::NotFound);
    }

    std::vector<ItemWearingFiles> out;
    out.reserve(keepOrder.size());
    for (const size_t index : keepOrder) { out.push_back(results[index]); }
    return Result<std::vector<ItemWearingFiles>>(std::move(out));
}

} // namespace Modern::Client