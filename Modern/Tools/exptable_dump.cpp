// exptable_dump.cpp
// Minimal, portable analyzer for RAN's exptable_max.bin.
//
// Legacy: GLCONST_CHAR::EXP_MAX_LOADFILE (GLogicDataLoad.cpp) reads exptable_max.bin
// line by line as 64-bit integers through CSerialFile + BYTECRYPT::byte_decode.
// This tool reproduces those byte-decode tables ONLY (no MFC/Win32/DirectX) to
// inspect the real data file for the Modern progression boundary. Does NOT modify the file.

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

typedef unsigned char UCHAR;
#include "../../Lib_Engine/Common/ByteCryptDef.h"
#include "../../Lib_Engine/Common/ByteCryptDefVer1.h"
#include "../../Lib_Engine/Common/ByteCryptDefVer2.h"

using U8 = unsigned char;

static const U8* g_arrays[42];

struct Candidate
{
    double score;
    int    cid;
    int    headerSkip;
    int    intLines;
    std::string decoded;
};

static void build_tables()
{
    static const char* names[42] = {
        "NONE", "OLD", "RAN2", "LEVEL", "WLD", "CROWSALE", "NPCTALK", "QUEST",
        "ITEM", "CROW", "CLASSCONST", "PETCHARSET", "SKILL", "CHARSET",
        "CONTAINER", "PIECE", "ITEMIX", "CFG", "EGP", "BIN", "TAXISTATION",
        "MAPSLIST", "LEVEL2", "WLD2", "CROWSALE2", "NPCTALK2", "QUEST2", "ITEM2",
        "CROW2", "CLASSCONST2", "PETCHARSET2", "SKILL2", "CHARSET2",
        "CONTAINER2", "PIECE2", "ITEMIX2", "CFG2", "EGP2", "BIN2",
        "TAXISTATION2", "MAPSLIST2", "ACTIVITY"
    };
    static const U8* tables[42] = {
        ARRAY_NONE, ARRAY_OLD, ARRAY_RAN2, ARRAY_LEVEL_VAR1, ARRAY_WLD_VAR1,
        ARRAY_CROWSALE_VAR1, ARRAY_NPCTALK_VAR1, ARRAY_QUEST_VAR1, ARRAY_ITEM_VAR1,
        ARRAY_CROW_VAR1, ARRAY_CLASSCONST_VAR1, ARRAY_PETCHARSET_VAR1, ARRAY_SKILL_VAR1,
        ARRAY_CHARSET_VAR1, ARRAY_CONTAINER_VAR1, ARRAY_PIECE_VAR1, ARRAY_ITEMIX_VAR1,
        ARRAY_CFG_VAR1, ARRAY_EGP_VAR1, ARRAY_BIN_VAR1, ARRAY_TAXISTATION_VAR1,
        ARRAY_MAPSLIST_VAR1, ARRAY_LEVEL_VER2, ARRAY_WLD_VER2, ARRAY_CROWSALE_VER2,
        ARRAY_NPCTALK_VER2, ARRAY_QUEST_VER2, ARRAY_ITEM_VER2, ARRAY_CROW_VER2,
        ARRAY_CLASSCONST_VER2, ARRAY_PETCHARSET_VER2, ARRAY_SKILL_VER2,
        ARRAY_CHARSET_VER2, ARRAY_CONTAINER_VER2, ARRAY_PIECE_VER2, ARRAY_ITEMIX_VER2,
        ARRAY_CFG_VER2, ARRAY_EGP_VER2, ARRAY_BIN_VER2, ARRAY_TAXISTATION_VER2,
        ARRAY_MAPSLIST_VER2, ARRAY_ACTIVITY
    };
    for (int i = 0; i < 42; ++i) g_arrays[i] = tables[i];
    (void)names;
}

static void invert(const U8* enc, U8* dec)
{
    for (int i = 0; i < 256; ++i)
        dec[enc[i]] = static_cast<U8>(i);
}

static std::string decode(const std::vector<U8>& raw, const U8* table)
{
    U8 dec[256];
    invert(table, dec);
    std::string out;
    out.reserve(raw.size());
    for (size_t i = 0; i < raw.size(); ++i)
        out.push_back(static_cast<char>(dec[raw[i]]));
    return out;
}

static double score_text(const std::string& s)
{
    if (s.empty()) return 0.0;
    int good = 0;
    for (unsigned char c : s)
    {
        if (c == '\r' || c == '\n' || c == '\t') { good += 2; continue; }
        if ((c >= '0' && c <= '9') || c == '-') { good += 2; continue; }
        if (c >= 32 && c < 127) { good += 1; continue; }
        good -= 3;
    }
    return (double)good / (double)s.size();
}

static int count_int_lines(const std::string& s)
{
    int n = 0;
    bool seen = false;
    for (size_t i = 0; i < s.size(); ++i)
    {
        char c = s[i];
        if (c == '\n')
        {
            if (seen) ++n;
            seen = false;
        }
        else if (c >= '0' && c <= '9' || c == '-')
        {
            seen = true;
        }
    }
    if (seen) ++n;
    return n;
}

int main(int argc, char** argv)
{
    const char* path = argc > 1 ? argv[1] : "exptable_max.bin";
    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        fprintf(stderr, "cannot open %s\n", path);
        return 1;
    }
    std::vector<U8> buf((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    fprintf(stdout, "file size: %zu bytes\n", buf.size());

    build_tables();

    std::vector<Candidate> cand;

    const int headers[] = {0, 4, 132};
    for (int hdr : headers)
    {
        std::vector<U8> payload(buf.begin() + hdr, buf.end());
        for (int id = 0; id < 42; ++id)
        {
            std::string d = decode(payload, g_arrays[id]);
            double sc = score_text(d);
            int lines = count_int_lines(d);
            cand.push_back({sc, id, hdr, lines, d});
        }
    }

    // Rank by score for preview (most ASCII-looking decodes first).
    std::sort(cand.begin(), cand.end(),
              [](const Candidate& a, const Candidate& b)
              { return a.score > b.score; });

    fprintf(stdout, "\ntop candidates by score:\n");
    int printed = 0;
    for (const auto& c : cand)
    {
        if (printed >= 15) break;
        std::string preview = c.decoded.substr(0, 240);
        for (auto& ch : preview) if (ch < 32 && ch != '\n' && ch != '\r') ch = '.';
        fprintf(stdout, "crypt=%d hdr=%d score=%.3f intlines=%d preview=\"%s\"\n",
                c.cid, c.headerSkip, c.score, c.intLines, preview.c_str());
        ++printed;
    }

    // Highest integer-line-count candidate with a healthy score is the table.
    std::sort(cand.begin(), cand.end(),
              [](const Candidate& a, const Candidate& b)
              { return a.intLines > b.intLines; });
    Candidate best{};
    int bestLines = 0;
    for (const auto& c : cand)
    {
        if (c.intLines >= 50 && c.score > 1.8 && c.intLines > bestLines)
        {
            best = c; bestLines = c.intLines;
        }
    }
    bool haveBest = bestLines > 0;
    if (!haveBest && !cand.empty())
    {
        // fall back to highest score
        best = cand.front();
        for (const auto& c : cand)
            if (c.score > best.score) best = c;
        haveBest = true;
    }

    if (haveBest)
    {
        fprintf(stdout, "\n=== BEST (crypt=%d hdr=%d intlines=%d score=%.2f) ===\n",
                best.cid, best.headerSkip, best.intLines, best.score);
        const std::string& s = best.decoded;
        int show = std::min(20, (int)best.intLines);
        int n = 0;
        std::string line;
        std::vector<std::string> out;
        for (size_t i = 0; i < s.size() && n < show; ++i)
        {
            char c = s[i];
            if (c == '\n')
            {
                out.push_back(line); line.clear(); n++;
            }
            else if (c != '\r')
            {
                line.push_back(c);
            }
        }
        for (const auto& l : out)
            fprintf(stdout, "L: %s\n", l.c_str());
        // first 5 and last 5
        fprintf(stdout, "\nfirst 5 lines:\n");
        n = 0; line.clear();
        for (size_t i = 0; i < s.size(); ++i)
        {
            char c = s[i];
            if (c == '\n')
            {
                if (n < 5) fprintf(stdout, "[%d] %s\n", n, line.c_str());
                line.clear(); n++;
                if (n >= 5) break;
            }
            else if (c != '\r') line.push_back(c);
        }
        // last lines
        std::vector<std::string> all;
        line.clear();
        for (size_t i = 0; i < s.size(); ++i)
        {
            char c = s[i];
            if (c == '\n') { all.push_back(line); line.clear(); }
            else if (c != '\r') line.push_back(c);
        }
        if (!line.empty()) all.push_back(line);
        fprintf(stdout, "total decoded int lines: %d\n", (int)all.size());
        if (!all.empty())
        {
            int start = (int)all.size() - 5; if (start < 0) start = 0;
            fprintf(stdout, "last 5 lines:\n");
            for (int i = start; i < (int)all.size(); ++i)
                fprintf(stdout, "[%d] %s\n", i, all[i].c_str());
        }
    }
    else
    {
        fprintf(stdout, "no high-confidence candidate; inspect top list above.\n");
    }

    return 0;
}
