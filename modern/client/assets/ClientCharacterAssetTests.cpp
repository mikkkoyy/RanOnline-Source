// ASURA-CHARACTER-001B: tests for the original RAN character asset path.
//
// Three units, one suite, because all three are the same question asked in
// three places -- "what did the real RAN character files actually say?":
//
//   ItemWearingFileReader   item.csv  -> the .cps wearing file per class slot
//   SkinPieceContainer      .cps      -> mesh / skeleton / mesh-node name
//   RanMaterialSlot         a material-texture name -> texture or RAN slot?
//
// Everything here is either built in this file or read from a real client
// tree when RAN_ASSET_ROOT names one. No proprietary bytes are committed, and
// the synthetic fixtures encode the *layout*, which is published in the
// legacy headers these readers were written against.
//
// The real-asset cases are opt-in and assert nothing about a tree that is not
// there: they skip (and report the skip) when RAN_ASSET_ROOT is unset, exactly
// as the existing DDS/PNG/XMesh real-asset cases do.

#include "TestHarness.h"

#include "assets/ItemWearingFile.h"
#include "assets/RanMaterialSlot.h"
#include "assets/SkinPieceContainer.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

namespace
{

// ---------------------------------------------------------------------------
// Optional real client tree
// ---------------------------------------------------------------------------

std::filesystem::path RealAssetRoot()
{
    #if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    const char* root = std::getenv("RAN_ASSET_ROOT");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    if (root == nullptr || root[0] == '\0') { return {}; }
    return std::filesystem::path(root);
}

void SkipRealAsset(const char* what)
{
    std::printf("    SKIP (no RAN_ASSET_ROOT): %s\n", what);
}

// ---------------------------------------------------------------------------
// item.csv fixture
// ---------------------------------------------------------------------------

// A miniature item.csv with the real header names this reader resolves by.
// Column order is deliberately NOT the shipped order: the reader must find
// the columns by name, so shuffling them is the test.
std::string ItemCsvFixture()
{
    std::string csv;
    csv += "strWearingFileRight 0,sStrName,sNativeID wSubID,strWearingFileRight 2,"
           "strWearingFileLeft 1,strWearingFileRight 1,sNativeID wMainID,"
           "strWearingFileRight 3,sComment\n";

    // The four confirmed Male Brawler starter items. The shipped rows give
    // Brawler Male (EMCHARINDEX 0) CUN0001_m.cps / CDN0001_m.cps /
    // CFN0034_m.cps / BTN0011_m.cps, and Archer Female (EMCHARINDEX 2) the
    // _W.cps variants.
    csv += "CUN0001_m.cps,\"Shirt, blue\",1,CUN0001_W.cps,CUN0001_L.cps,,50,,\n";
    csv += "CDN0001_m.cps,\"Pants, grey\",1,CDN0001_W.cps,CDN0001_L.cps,,53,,\n";
    csv += "CFN0034_m.cps,Boots,1,CFN0034_W.cps,CFN0034_L.cps,,56,,\n";
    csv += "BTN0011_m.cps,Rod,1,BTN0011_W.cps,BTN0011_L.cps,,23,,\n";

    // A row with a quoted field containing the delimiter and a doubled quote,
    // to pin the quoting rules.
    csv += "X0001.cps,\"He said \"\"hi\"\", loudly\",7,X0001_W.cps,,,777,,\n";

    // A short row (malformed: fewer fields than the id columns need).
    csv += "TOOSHORT_MISSING_COLUMNS\n";

    // A non-numeric id.
    csv += "BAD.cps,Bad,notanumber,,BAD_W.cps,,888,,\n";

    return csv;
}

struct TempTree
{
    std::filesystem::path root;
    explicit TempTree(const char* tag)
    {
        root = std::filesystem::temp_directory_path() /
               ("modern_char_001b_" + std::string(tag));
        std::error_code ec;
        std::filesystem::remove_all(root, ec);
        std::filesystem::create_directories(root, ec);
    }
    ~TempTree()
    {
        std::error_code ec;
        std::filesystem::remove_all(root, ec);
    }
    void Write(const std::string& relative, const std::string& contents) const
    {
        std::ofstream out(root / relative, std::ios::binary | std::ios::trunc);
        out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    }
};

// ---------------------------------------------------------------------------
// .cps fixture
// ---------------------------------------------------------------------------

void AppendU32(std::vector<uint8_t>& out, uint32_t value)
{
    out.push_back(static_cast<uint8_t>(value & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void AppendString(std::vector<uint8_t>& out, const std::string& value)
{
    AppendU32(out, static_cast<uint32_t>(value.size() + 1));  // count includes the NUL
    out.insert(out.end(), value.begin(), value.end());
    out.push_back(0);
}

// A minimal, well-formed version 0x0106 container with an opaque tail.
// Field order is the legacy LoadPiece_0106 order:
//   version, reference, mesh, skeleton, pieceType, weaponWhereBack,
//   meshNodeName, materialSlotCount, <tail>
std::vector<uint8_t> SkinPieceFixture(uint32_t meshNodeSlotCount = 2)
{
    std::vector<uint8_t> out;
    // 128-byte container tag, then the 4-byte file id.
    const char* tag = "default";
    out.insert(out.end(), tag, tag + 7);
    out.insert(out.end(), 128 - 7, uint8_t{0});
    AppendU32(out, 0);  // file id

    AppendU32(out, SkinPieceContainer::kSupportedVersion);
    AppendU32(out, 0);                       // reference
    AppendString(out, "s_m_hb.X");           // mesh
    AppendString(out, "b_m.X");              // skeleton
    AppendU32(out, 1);                       // piece type: UPPER
    AppendU32(out, 0);                       // weapon where back
    AppendString(out, "m2_hb_body[Mesh]");   // mesh node name
    AppendU32(out, meshNodeSlotCount);       // material slot count

    // Opaque tail: stands in for the material / trace / effect arrays this
    // reader deliberately does not decode.
    for (uint32_t i = 0; i < meshNodeSlotCount; ++i)
    {
        AppendU32(out, 0x0100);
        AppendU32(out, 8);
        AppendU32(out, 9);   // strlen("ARROW01") + 1
        AppendString(out, "ARROW01");
    }
    return out;
}

ResourceData ToResourceData(const std::vector<uint8_t>& bytes)
{
    return ResourceData(bytes);
}

} // namespace

// ===========================================================================
// ItemWearingFileReader -- CSV mechanics
// ===========================================================================

MODERN_TEST(ItemCsv_SplitsPlainRecord)
{
    const Result<std::vector<std::string>> fields = SplitCsvRecord("a,b,c");
    REQUIRE(fields.IsOk());
    CHECK_EQ(fields.GetValue().size(), size_t{3});
    CHECK_EQ(fields.GetValue()[0], std::string("a"));
    CHECK_EQ(fields.GetValue()[2], std::string("c"));
}

MODERN_TEST(ItemCsv_SplitsQuotedFieldContainingDelimiter)
{
    const Result<std::vector<std::string>> fields = SplitCsvRecord("a,\"b,c\",d");
    REQUIRE(fields.IsOk());
    CHECK_EQ(fields.GetValue().size(), size_t{3});
    CHECK_EQ(fields.GetValue()[1], std::string("b,c"));
}

MODERN_TEST(ItemCsv_SplitsDoubledQuoteEscape)
{
    const Result<std::vector<std::string>> fields = SplitCsvRecord("x,\"he said \"\"hi\"\"\",y");
    REQUIRE(fields.IsOk());
    CHECK_EQ(fields.GetValue().size(), size_t{3});
    CHECK_EQ(fields.GetValue()[1], std::string("he said \"hi\""));
}

MODERN_TEST(ItemCsv_PreservesEmptyFields)
{
    const Result<std::vector<std::string>> fields = SplitCsvRecord("a,,c,");
    REQUIRE(fields.IsOk());
    CHECK_EQ(fields.GetValue().size(), size_t{4});
    CHECK_EQ(fields.GetValue()[1], std::string(""));
    CHECK_EQ(fields.GetValue()[3], std::string(""));
}

MODERN_TEST(ItemCsv_UnterminatedQuoteIsRefused)
{
    CHECK(SplitCsvRecord("a,\"unterminated,b").IsError());
}

// ===========================================================================
// ItemWearingFileReader -- extraction
// ===========================================================================

MODERN_TEST(ItemWearingFile_ExtractsTheFourConfirmedClass00Items)
{
    TempTree tree("items_ok");
    tree.Write("item.csv", ItemCsvFixture());

    ItemWearingFileReader reader;
    REQUIRE(reader.Initialize(tree.root / "item.csv").IsOk());

    // Exactly the ids class00.charset puts on the Male Brawler.
    const std::vector<NativeId> wanted = { {50, 1}, {53, 1}, {56, 1}, {23, 1} };

    ItemWearingFileReport report;
    const Result<std::vector<ItemWearingFiles>> rows = reader.Extract(wanted, report);
    REQUIRE(rows.IsOk());
    CHECK_EQ(rows.GetValue().size(), size_t{4});
    CHECK_EQ(report.rowsMatched, size_t{4});
    CHECK_EQ(report.missingIds.size(), size_t{0});
    CHECK_EQ(report.duplicateIds, size_t{0});

    // EMCHARINDEX 0 is GLCI_BRAWLER_M.
    CHECK_EQ(rows.GetValue()[0].ForClassIndex(0), std::string("CUN0001_m.cps"));
    CHECK_EQ(rows.GetValue()[1].ForClassIndex(0), std::string("CDN0001_m.cps"));
    CHECK_EQ(rows.GetValue()[2].ForClassIndex(0), std::string("CFN0034_m.cps"));
    CHECK_EQ(rows.GetValue()[3].ForClassIndex(0), std::string("BTN0011_m.cps"));
}

MODERN_TEST(ItemWearingFile_MapsEmclassindexToRightAndLeftColumns)
{
    TempTree tree("items_slots");
    tree.Write("item.csv", ItemCsvFixture());

    ItemWearingFileReader reader;
    REQUIRE(reader.Initialize(tree.root / "item.csv").IsOk());

    ItemWearingFileReport report;
    const Result<std::vector<ItemWearingFiles>> rows =
        reader.Extract({ {50, 1} }, report);
    REQUIRE(rows.IsOk());

    const ItemWearingFiles& row = rows.GetValue().front();
    // EMCHARINDEX 0 -> strWearingFileRight 0.
    CHECK_EQ(row.right[0], std::string("CUN0001_m.cps"));
    // EMCHARINDEX 2 -> strWearingFileRight 2.
    CHECK_EQ(row.right[2], std::string("CUN0001_W.cps"));
    // EMCHARINDEX 1 -> strWearingFileLeft 1 (right 1 is empty in the fixture).
    CHECK(row.right[1].empty());
    CHECK_EQ(row.left[1], std::string("CUN0001_L.cps"));
    // ForClassIndex falls back to the left hand.
    CHECK_EQ(row.ForClassIndex(1), std::string("CUN0001_L.cps"));
    // ForClassIndex prefers the right hand when both hands declare the slot.
    CHECK_EQ(row.ForClassIndex(2), std::string("CUN0001_W.cps"));
}

MODERN_TEST(ItemWearingFile_HandlesQuotedCommaAndEscapedQuoteInRow)
{
    TempTree tree("items_quoted");
    tree.Write("item.csv", ItemCsvFixture());

    ItemWearingFileReader reader;
    REQUIRE(reader.Initialize(tree.root / "item.csv").IsOk());

    ItemWearingFileReport report;
    const Result<std::vector<ItemWearingFiles>> rows =
        reader.Extract({ {777, 7} }, report);
    REQUIRE(rows.IsOk());
    // The row that carries the comma and the escaped quote was still split
    // correctly, so the id and the wearing-file columns were found.
    CHECK_EQ(rows.GetValue().front().ForClassIndex(0), std::string("X0001.cps"));
}

MODERN_TEST(ItemWearingFile_MissingIdIsReportedNotGuessed)
{
    TempTree tree("items_missing");
    tree.Write("item.csv", ItemCsvFixture());

    ItemWearingFileReader reader;
    REQUIRE(reader.Initialize(tree.root / "item.csv").IsOk());

    ItemWearingFileReport report;
    const Result<std::vector<ItemWearingFiles>> rows =
        reader.Extract({ {50, 1}, {999999, 1} }, report);
    // Partial success: the found row is returned, the missing id is reported.
    REQUIRE(rows.IsOk());
    CHECK_EQ(rows.GetValue().size(), size_t{1});
    CHECK_EQ(report.missingIds.size(), size_t{1});
    CHECK_EQ(report.missingIds[0].mainId, uint32_t{999999});
}

MODERN_TEST(ItemWearingFile_AllIdsMissingIsNotFound)
{
    TempTree tree("items_none");
    tree.Write("item.csv", ItemCsvFixture());

    ItemWearingFileReader reader;
    REQUIRE(reader.Initialize(tree.root / "item.csv").IsOk());

    ItemWearingFileReport report;
    CHECK(reader.Extract({ {424242, 9} }, report).IsError());
    CHECK_EQ(report.missingIds.size(), size_t{1});
}

MODERN_TEST(ItemWearingFile_DuplicateRowsAreCounted)
{
    TempTree tree("items_dup");
    std::string csv = ItemCsvFixture();
    csv += "CUN0001_m.cps,Duplicate,1,,,,50,,\n";
    tree.Write("item.csv", csv);

    ItemWearingFileReader reader;
    REQUIRE(reader.Initialize(tree.root / "item.csv").IsOk());

    ItemWearingFileReport report;
    const Result<std::vector<ItemWearingFiles>> rows =
        reader.Extract({ {50, 1} }, report);
    REQUIRE(rows.IsOk());
    CHECK_EQ(rows.GetValue().size(), size_t{1});
    CHECK_EQ(report.duplicateIds, size_t{1});
}

MODERN_TEST(ItemWearingFile_MalformedRowsAreCountedNotFatal)
{
    TempTree tree("items_malformed");
    tree.Write("item.csv", ItemCsvFixture());

    ItemWearingFileReader reader;
    REQUIRE(reader.Initialize(tree.root / "item.csv").IsOk());

    ItemWearingFileReport report;
    const Result<std::vector<ItemWearingFiles>> rows =
        reader.Extract({ {50, 1} }, report);
    REQUIRE(rows.IsOk());
    // The short row and the non-numeric-id row are both counted, and neither
    // prevented the good rows from being read.
    CHECK_GE(report.rowsMalformed, size_t{2});
    CHECK_EQ(rows.GetValue().size(), size_t{1});
}

MODERN_TEST(ItemWearingFile_MissingHeaderColumnIsRefused)
{
    TempTree tree("items_badheader");
    // No "sNativeID wMainID" column.
    tree.Write("item.csv", "strWearingFileRight 0,strWearingFileRight 1\na,b\n");

    ItemWearingFileReader reader;
    CHECK(reader.Initialize(tree.root / "item.csv").IsError());
}

MODERN_TEST(ItemWearingFile_MissingFileIsNotFound)
{
    ItemWearingFileReader reader;
    CHECK(reader.Initialize(std::filesystem::path("no_such_item.csv")).IsError());
    CHECK(reader.Initialize(std::filesystem::path()).IsError());
}

MODERN_TEST(ItemWearingFile_DuplicateRequestIsRefused)
{
    TempTree tree("items_dupreq");
    tree.Write("item.csv", ItemCsvFixture());

    ItemWearingFileReader reader;
    REQUIRE(reader.Initialize(tree.root / "item.csv").IsOk());

    ItemWearingFileReport report;
    CHECK(reader.Extract({ {50, 1}, {50, 1} }, report).IsError());
    CHECK(reader.Extract({}, report).IsError());
}

MODERN_TEST(ItemWearingFile_ExtractBeforeInitializeIsInvalidState)
{
    ItemWearingFileReader reader;
    ItemWearingFileReport report;
    CHECK(reader.Extract({ {1, 1} }, report).IsError());
}

MODERN_TEST(ItemWearingFile_DoesNotMatchPartialIds)
{
    TempTree tree("items_prefix");
    tree.Write("item.csv", ItemCsvFixture());

    ItemWearingFileReader reader;
    REQUIRE(reader.Initialize(tree.root / "item.csv").IsOk());

    ItemWearingFileReport report;
    // 5 must not match 50, and 11 must not match 1.
    CHECK(reader.Extract({ {5, 1}, {50, 11} }, report).IsError());
}

MODERN_TEST(ItemWearingFile_RealAsuraClass00StarterGear)
{
    const std::filesystem::path root = RealAssetRoot();
    if (root.empty())
    {
        SkipRealAsset("item.csv wearing files for the class00 starter gear");
        return;
    }
    const std::filesystem::path csv = root / "data" / "glogic" / "item.csv";
    if (!std::filesystem::is_regular_file(csv))
    {
        SkipRealAsset("item.csv not present under RAN_ASSET_ROOT");
        return;
    }

    ItemWearingFileReader reader;
    REQUIRE(reader.Initialize(csv).IsOk());
    CHECK_GE(reader.GetRightColumnCount(), size_t{16});
    CHECK_GE(reader.GetLeftColumnCount(), size_t{16});

    ItemWearingFileReport report;
    const Result<std::vector<ItemWearingFiles>> rows =
        reader.Extract({ {50, 1}, {53, 1}, {56, 1}, {23, 1} }, report);
    REQUIRE(rows.IsOk());
    CHECK_EQ(rows.GetValue().size(), size_t{4});
    CHECK_EQ(report.missingIds.size(), size_t{0});

    // These four are the values ASURA-CHARACTER-001A established by reading
    // the real file; they are pinned here so a change in the shipped data is
    // a visible test failure rather than a silent behaviour change.
    CHECK_EQ(rows.GetValue()[0].ForClassIndex(0), std::string("CUN0001_m.cps"));
    CHECK_EQ(rows.GetValue()[1].ForClassIndex(0), std::string("CDN0001_m.cps"));
    CHECK_EQ(rows.GetValue()[2].ForClassIndex(0), std::string("CFN0034_m.cps"));
    CHECK_EQ(rows.GetValue()[3].ForClassIndex(0), std::string("BTN0011_m.cps"));

    // The same rows give the Archer Female (EMCHARINDEX 2) the _W.cps pieces,
    // which is the class/gender mapping the item table encodes.
    CHECK_EQ(rows.GetValue()[0].ForClassIndex(2), std::string("CUN0001_W.cps"));
}

// ===========================================================================
// SkinPieceContainer -- synthetic layout
// ===========================================================================

MODERN_TEST(SkinPiece_DecodesTheConfirmedPrefix)
{
    const Result<SkinPiece> piece =
        SkinPieceContainer::Decode(ToResourceData(SkinPieceFixture()));
    REQUIRE(piece.IsOk());

    const SkinPiece& p = piece.GetValue();
    CHECK_EQ(p.version, SkinPieceContainer::kSupportedVersion);
    CHECK_EQ(p.fileType, std::string("default"));
    CHECK_EQ(p.fileId, uint32_t{0});
    CHECK_EQ(p.reference, uint32_t{0});
    CHECK_EQ(p.meshFilename, std::string("s_m_hb.X"));
    CHECK_EQ(p.skeletonFilename, std::string("b_m.X"));
    CHECK_EQ(p.pieceTypeRaw, uint32_t{1});
    CHECK_EQ(p.weaponWhereBack, uint32_t{0});
    CHECK_EQ(p.meshNodeName, std::string("m2_hb_body[Mesh]"));
    CHECK_EQ(p.materialSlotCount, uint32_t{2});
    CHECK(p.GetPieceType() == SkinPieceType::Upper);
}

MODERN_TEST(SkinPiece_OpaqueTailIsPreservedNotReinterpreted)
{
    const std::vector<uint8_t> bytes = SkinPieceFixture(2);
    const Result<SkinPiece> piece = SkinPieceContainer::Decode(ToResourceData(bytes));
    REQUIRE(piece.IsOk());

    const SkinPiece& p = piece.GetValue();
    // Everything after materialSlotCount is kept byte for byte.
    CHECK_EQ(p.GetOpaqueTailSize(), bytes.size() - p.opaqueTailOffset);
    const uint8_t* tail = p.opaqueTail.data();
    const size_t   offset = p.opaqueTailOffset;
    CHECK(tail != nullptr);
    if (tail != nullptr && offset < bytes.size())
    {
        CHECK_EQ(static_cast<int>(tail[0]), static_cast<int>(bytes[offset]));
    }
}

MODERN_TEST(SkinPiece_EmptyInputIsRefused)
{
    CHECK(SkinPieceContainer::Decode(ResourceData()).IsError());
    CHECK(SkinPieceContainer::Decode(ToResourceData(std::vector<uint8_t>(64, 0))).IsError());
}

MODERN_TEST(SkinPiece_UnsupportedVersionIsNotAllowed)
{
    std::vector<uint8_t> bytes = SkinPieceFixture();
    // Version lives right after the 132-byte container header.
    bytes[132] = 0x07;
    bytes[133] = 0x01;
    const Result<SkinPiece> piece = SkinPieceContainer::Decode(ToResourceData(bytes));
    REQUIRE(piece.IsError());
    CHECK(piece.GetError() == ErrorCode::NotAllowed);
}

MODERN_TEST(SkinPiece_TruncatedAtEveryLengthIsRefusedOrDecodes)
{
    const std::vector<uint8_t> full = SkinPieceFixture();
    for (size_t length = 0; length < full.size(); ++length)
    {
        const Result<SkinPiece> piece =
            SkinPieceContainer::Decode(ToResourceData(std::vector<uint8_t>(full.begin(),
                                                                             full.begin() +
                                                                                 static_cast<long>(length))));
        // A prefix may legitimately decode (the opaque tail can be empty), but
        // it must never claim a mesh it did not read.
        if (piece.IsOk())
        {
            CHECK(!piece.GetValue().meshFilename.empty());
            CHECK(piece.GetValue().opaqueTailOffset <= length);
        }
    }
}

MODERN_TEST(SkinPiece_TruncatedBeforeMeshNameIsRefused)
{
    std::vector<uint8_t> bytes = SkinPieceFixture();
    bytes.resize(140);  // header + version + reference, no filename length
    CHECK(SkinPieceContainer::Decode(ToResourceData(bytes)).IsError());
}

MODERN_TEST(SkinPiece_RunawayStringLengthIsRefused)
{
    std::vector<uint8_t> bytes = SkinPieceFixture();
    // The first string length is at 132 + 4 (version) + 4 (reference).
    const size_t lengthOffset = SkinPieceContainer::kHeaderSize + 8;
    bytes[lengthOffset + 0] = 0xFF;
    bytes[lengthOffset + 1] = 0xFF;
    bytes[lengthOffset + 2] = 0xFF;
    bytes[lengthOffset + 3] = 0x7F;
    CHECK(SkinPieceContainer::Decode(ToResourceData(bytes)).IsError());
}

MODERN_TEST(SkinPiece_ZeroLengthStringIsRefused)
{
    std::vector<uint8_t> bytes = SkinPieceFixture();
    const size_t lengthOffset = SkinPieceContainer::kHeaderSize + 8;
    bytes[lengthOffset + 0] = 0;
    bytes[lengthOffset + 1] = 0;
    bytes[lengthOffset + 2] = 0;
    bytes[lengthOffset + 3] = 0;
    CHECK(SkinPieceContainer::Decode(ToResourceData(bytes)).IsError());
}

MODERN_TEST(SkinPiece_ImplausibleMaterialCountIsRefused)
{
    std::vector<uint8_t> bytes = SkinPieceFixture();

    // The 0x0106 payload order is:
    //   version, reference, mesh, skeleton, pieceType, weaponWhereBack,
    //   meshNodeName, materialSlotCount
    // so the two uint32s sit BETWEEN the second and third string, not after
    // all three.
    size_t cursor = SkinPieceContainer::kHeaderSize + 4 + 4;
    for (int stringIndex = 0; stringIndex < 3; ++stringIndex)
    {
        if (stringIndex == 2) { cursor += 8; }  // pieceType + weaponWhereBack
        const uint32_t length = static_cast<uint32_t>(bytes[cursor]) |
                                (static_cast<uint32_t>(bytes[cursor + 1]) << 8) |
                                (static_cast<uint32_t>(bytes[cursor + 2]) << 16) |
                                (static_cast<uint32_t>(bytes[cursor + 3]) << 24);
        cursor += 4 + length;
    }

    // Overwrite the material slot count with an absurd value.
    bytes[cursor + 0] = 0xFF;
    bytes[cursor + 1] = 0xFF;
    bytes[cursor + 2] = 0xFF;
    bytes[cursor + 3] = 0xFF;
    CHECK(SkinPieceContainer::Decode(ToResourceData(bytes)).IsError());
}

MODERN_TEST(SkinPiece_PieceTypeNamesAreStable)
{
    CHECK_EQ(std::string(ToString(SkinPieceType::Upper)), std::string("UPPER"));
    CHECK_EQ(std::string(ToString(SkinPieceType::Lower)), std::string("LOWER"));
    CHECK_EQ(std::string(ToString(SkinPieceType::Hand)),  std::string("RHAND"));
    CHECK_EQ(std::string(ToString(SkinPieceType::Foot)),  std::string("FOOT"));
    CHECK_EQ(std::string(ToString(SkinPieceType::Unknown)), std::string("UNKNOWN"));
}

// ===========================================================================
// SkinPieceContainer -- the four real ASURA class00 pieces
// ===========================================================================

MODERN_TEST(SkinPiece_RealAsuraClass00PiecesDecodeToTheirMeshes)
{
    const std::filesystem::path root = RealAssetRoot();
    if (root.empty())
    {
        SkipRealAsset("the four class00 .cps wearing files");
        return;
    }
    const std::filesystem::path skinObject = root / "data" / "skinobject";
    if (!std::filesystem::is_directory(skinObject))
    {
        SkipRealAsset("data/skinobject not present under RAN_ASSET_ROOT");
        return;
    }

    struct Expectation
    {
        const char* file;
        const char* mesh;
        const char* skeleton;
        const char* node;
        uint32_t    pieceType;
    };
    // Every value here was read out of the real files by ASURA-CHARACTER-001A.
    const Expectation expected[] = {
        { "CUN0001_m.cps", "s_m_hb.X",      "b_m.X", "m2_hb_body[Mesh]", 1 },
        { "CDN0001_m.cps",  "s_m_hb.X",      "b_m.X", "m2_hb_leg[Mesh]",  2 },
        { "CFN0034_m.cps",  "s_m_hb.X",      "b_m.X", "m2_hb_foot[Mesh]", 6 },
        { "BTN0011_m.cps",  "s_m_gongyong.X","b_m.X", "BTN0011[Mesh]",    4 },
    };

    for (const Expectation& e : expected)
    {
        const std::filesystem::path path = skinObject / e.file;
        if (!std::filesystem::is_regular_file(path))
        {
            std::printf("    SKIP: %s absent\n", e.file);
            continue;
        }
        std::ifstream in(path, std::ios::binary);
        REQUIRE(in);
        const std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)),
                                         std::istreambuf_iterator<char>());

        const Result<SkinPiece> piece = SkinPieceContainer::Decode(ToResourceData(bytes));
        REQUIRE(piece.IsOk());
        const SkinPiece& p = piece.GetValue();

        CHECK_EQ(p.version, SkinPieceContainer::kSupportedVersion);
        CHECK_EQ(p.meshFilename, std::string(e.mesh));
        CHECK_EQ(p.skeletonFilename, std::string(e.skeleton));
        CHECK_EQ(p.meshNodeName, std::string(e.node));
        CHECK_EQ(p.pieceTypeRaw, e.pieceType);

        // The mesh must actually exist in the tree, or the piece names
        // something this client cannot draw.
        const std::filesystem::path meshPath = root / "data" / "skin" / p.meshFilename;
        std::printf("    %s -> %s / %s / %s (mesh exists: %s)\n",
                    e.file, p.meshFilename.c_str(), p.skeletonFilename.c_str(),
                    p.meshNodeName.c_str(),
                    std::filesystem::is_regular_file(meshPath) ? "yes" : "NO");
        CHECK(std::filesystem::is_regular_file(meshPath));
    }
}

// ===========================================================================
// RanMaterialSlot classification
// ===========================================================================

MODERN_TEST(RanMaterialSlot_RealHbBodyTextureNamesAreFilenames)
{
    // The five TextureFilename values s_m_hb.X actually declares.
    CHECK(ClassifyRanMaterialName("m1_hb_body.dds")  == RanMaterialNameKind::TextureFileName);
    CHECK(ClassifyRanMaterialName("m2_bs_body.DDS")  == RanMaterialNameKind::TextureFileName);
    CHECK(ClassifyRanMaterialName("m1_khb_foot.dds") == RanMaterialNameKind::TextureFileName);
    CHECK(ClassifyRanMaterialName("m1_hb_leg.dds")   == RanMaterialNameKind::TextureFileName);
    CHECK(ClassifyRanMaterialName("m1_hb_hand.dds")  == RanMaterialNameKind::TextureFileName);
}

MODERN_TEST(RanMaterialSlot_RealClass00SlotNamesAreRanSlots)
{
    // Observed in the four confirmed class00 .cps files.
    const char* slots[] = { "IMAGE01", "IMAGE02", "IMAGE11", "IMAGE12", "IMAGE21",
                            "IMAGE22", "BLOOD01", "STRIKE01", "STRIKE02", "ARROW01" };
    for (const char* slot : slots)
    {
        CHECK(IsRanMaterialSlotName(slot));
        CHECK(ClassifyRanMaterialName(slot) == RanMaterialNameKind::RanMaterialSlot);
    }
}

MODERN_TEST(RanMaterialSlot_EmptyAndOddNamesAreClassified)
{
    CHECK(ClassifyRanMaterialName("") == RanMaterialNameKind::None);
    CHECK(ClassifyRanMaterialName("IMAGE") == RanMaterialNameKind::Unrecognized);
    CHECK(ClassifyRanMaterialName("IMAGE0X") == RanMaterialNameKind::Unrecognized);
    CHECK(ClassifyRanMaterialName("image01") == RanMaterialNameKind::Unrecognized);
    CHECK(ClassifyRanMaterialName("char/m1_hb_body.dds") == RanMaterialNameKind::Unrecognized);
    CHECK(ClassifyRanMaterialName("noextension") == RanMaterialNameKind::Unrecognized);
}

MODERN_TEST(RanMaterialSlot_KindsHaveStableNames)
{
    CHECK_EQ(std::string(ToString(RanMaterialNameKind::None)),
             std::string("none"));
    CHECK_EQ(std::string(ToString(RanMaterialNameKind::TextureFileName)),
             std::string("texture-filename"));
    CHECK_EQ(std::string(ToString(RanMaterialNameKind::RanMaterialSlot)),
             std::string("ran-material-slot"));
    CHECK_EQ(std::string(ToString(RanMaterialNameKind::Unrecognized)),
             std::string("unrecognized"));
}

MODERN_TEST(RanMaterialSlot_UnresolvedSlotIsReportedNotHidden)
{
    const std::string text =
        DescribeRanMaterialResolution("CUN0001_m.cps", "IMAGE01", "");
    CHECK(text.find("IMAGE01") != std::string::npos);
    CHECK(text.find("UNRESOLVED") != std::string::npos);
    CHECK(text.find("CUN0001_m.cps") != std::string::npos);
    // A slot must never be described as if it had resolved to a file.
    CHECK(text.find("resolved to") == std::string::npos);
}

MODERN_TEST(RanMaterialSlot_ResolvedFilenameReportsItsPath)
{
    const std::string text = DescribeRanMaterialResolution(
        "CUN0001_m.cps", "m1_hb_body.dds", "textures/char/m1_hb_body.dds");
    CHECK(text.find("textures/char/m1_hb_body.dds") != std::string::npos);
    CHECK(text.find("UNRESOLVED") == std::string::npos);
}

int main()
{
    std::printf("Modern ASURA-CHARACTER-001B character asset tests\n\n");

    const int failedCases = ModernTests::RunAll();

    if (failedCases == 0)
    {
        std::printf("\nAll %d test cases passed.\n",
                    static_cast<int>(ModernTests::Registry().size()));
        return 0;
    }

    std::printf("\n%d of %d test cases FAILED (%d checks).\n",
                failedCases,
                static_cast<int>(ModernTests::Registry().size()),
                ModernTests::FailureCount());
    return 1;
}
