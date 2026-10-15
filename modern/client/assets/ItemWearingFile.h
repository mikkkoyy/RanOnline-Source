#pragma once

// ASURA-CHARACTER-001B: the bounded item-wearing-file reader.
//
// RAN does not store a character's meshes in its class configuration. A
// playable character is *assembled from the equipment it wears*, and the
// class configuration only records which items it starts with. The chain
// that turns an item into geometry is:
//
//   data/glogic/classXX.charset   PutOnItems [UPPER] [50, 1]
//   -> data/glogic/item.csv       row (50, 1), column strWearingFileRight 0
//   -> data/skinobject/CUN0001_m.cps
//   -> data/skin/s_m_hb.X        (the mesh the piece actually draws)
//
// This class owns exactly the middle step: given one or more
// (wMainID, wSubID) pairs, it finds those rows in `item.csv` and returns
// the `strWearingFileRight` / `strWearingFileLeft` columns.
//
// Why a bounded reader and not a model:
//
//   `item.csv` is ~22 MB with 399 columns per row. A character's four
//   starter items need four columns from each of four rows. Parsing the
//   whole file into a general item model to obtain that would be a large,
//   speculative, and wrong-shaped dependency for the only thing the client
//   actually asks of this file. So this reader takes the ids it wants up
//   front, makes ONE pass over the file, and keeps only the matched rows'
//   wearing-file columns. Everything else is parsed past and discarded.
//
// Column identification is by header *name*, never by position:
//
//   The header is a real CSV row whose names are what the legacy loader
//   looks up (`gltexfile` builds a name->index map, and
//   `GLItemMan` reads these columns by name). Hardcoding column 38 would
//   silently read the wrong data if RAN ever reorders the file. Every
//   column this class needs is located by name, and a file whose header
//   lacks any required column is refused rather than guessed at.
//
// CSV correctness:
//
//   RAN's `item.csv` has quoted fields (several columns contain commas and
//   embedded quotes), so the parser is a real CSV field splitter: RFC 4180
//   doubled-quote escaping, quoted fields that may contain the delimiter
//   and newlines are not special here because RAN emits no embedded
//   newlines, and an unterminated quote ends the row as malformed rather
//   than swallowing the rest of the file.
//
// What it deliberately does NOT do:
//
//   - **No filename-pattern inference.** `CUN0001_m.cps` looks like it
//     encodes class and gender, and it is tempting to synthesise wearing
//     files from a naming convention. This reader never does that: a
//     returned value is always a literal column value from a literal row.
//   - **No fuzzy id matching.** Ids compare exactly, as two independent
//     unsigned decimals. `50` never matches `500`.
//   - **No partial results presented as success.** A requested id that has
//     no row, or a row that appears twice, is reported through `Report`
//     and through the returned error code; it is never quietly dropped.

#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Modern::Client
{

// The two halves of a RAN native id, as they appear in `item.csv`.
struct NativeId
{
    uint32_t mainId = 0;
    uint32_t subId  = 0;

    friend bool operator==(const NativeId& lhs, const NativeId& rhs) noexcept
    {
        return lhs.mainId == rhs.mainId && lhs.subId == rhs.subId;
    }

    friend bool operator!=(const NativeId& lhs, const NativeId& rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

// RAN addresses a worn piece per class/gender variant through 32 parallel
// columns: `strWearingFileRight 0..15` and `strWearingFileLeft 0..15`.
// The column index is the character's EMCHARINDEX (0..15) as declared by
// `legacy/Lib_Client/G-Logic/GLCharDefine.h:235` (`EMCHARINDEX`), so the
// same row yields a different wearing file per class.
inline constexpr size_t kWearingFileSlots = 16;

// The wearing-file columns of one matched `item.csv` row.
struct ItemWearingFiles
{
    NativeId  id;
    // `strWearingFileRight n` / `strWearingFileLeft n`, trimmed. An entry is
    // empty when the item declares no piece for that class slot.
    std::string right[kWearingFileSlots];
    std::string left[kWearingFileSlots];

    // The column value for one class index, preferring the right hand and
    // falling back to the left. Empty when the item declares neither.
    const std::string& ForClassIndex(size_t classIndex) const
    {
        return right[classIndex].empty() ? left[classIndex] : right[classIndex];
    }
};

// What the reader did, so a caller can tell "all four found" from "three
// found and one row was silently skipped".
struct ItemWearingFileReport
{
    size_t rowsScanned      = 0;  // data rows examined
    size_t rowsMatched      = 0;  // rows whose ids matched a requested id
    size_t rowsMalformed    = 0;  // rows refused: bad quoting, or wrong field count
    size_t duplicateIds     = 0;  // requested ids that matched more than one row
    std::vector<NativeId> missingIds;  // requested ids with no matching row
};

// Header names this reader requires. Exposed so tests and callers can talk
// about the contract instead of repeating string literals.
namespace ItemCsvColumns
{
    inline constexpr const char* kMainId      = "sNativeID wMainID";
    inline constexpr const char* kSubId       = "sNativeID wSubID";
    inline constexpr const char* kRightPrefix = "strWearingFileRight ";
    inline constexpr const char* kLeftPrefix  = "strWearingFileLeft ";
} // namespace ItemCsvColumns

// Splits one CSV record into fields. Exposed for testing: the reader's
// quoting behaviour is part of its contract, not an implementation detail.
//
//   InvalidArgument - the record has an unterminated quoted field
//
// `record` must not contain a newline; RAN's `item.csv` does not quote
// across lines, so an embedded newline is not a supported shape.
Result<std::vector<std::string>> SplitCsvRecord(const std::string& record);

// One pass over `item.csv`, keeping only the requested rows' wearing-file
// columns.
//
//   NotFound        - the file does not exist or is not a regular file
//   InvalidArgument - the header is missing a required column, or a
//                     requested id list is empty / carries a duplicate id
//
// On success the result holds one entry per requested id that had a row,
// in the order the ids were requested. `report` (out parameter, never null)
// receives the scan statistics including every missing and duplicate id.
class ItemWearingFileReader final
{
public:
    //   InvalidArgument - path is empty
    //   NotFound        - not a readable regular file
    Status Initialize(const std::filesystem::path& itemCsvPath);

    const std::filesystem::path& GetPath() const noexcept { return m_path; }
    bool IsInitialized() const noexcept { return m_initialized; }

    // Number of wearing-file columns the header actually declared, per hand.
    // A conforming RAN file declares 16 of each.
    size_t GetRightColumnCount() const noexcept { return m_rightColumns.size(); }
    size_t GetLeftColumnCount() const noexcept { return m_leftColumns.size(); }

    // Extracts the requested rows. Returns NotFound when *no* requested id
    // had a row; a partial success returns the rows it did find and reports
    // the rest as missing, so the caller decides whether that is fatal.
    Result<std::vector<ItemWearingFiles>> Extract(const std::vector<NativeId>& requested,
                                                  ItemWearingFileReport& report);

private:
    std::filesystem::path m_path;
    bool                  m_initialized = false;
    // Column indices resolved by header NAME at Initialize() time.
    size_t                        m_mainIdColumn = 0;
    size_t                        m_subIdColumn  = 0;
    std::vector<size_t>           m_rightColumns;  // indexed by EMCHARINDEX
    std::vector<size_t>           m_leftColumns;   // indexed by EMCHARINDEX
};

} // namespace Modern::Client