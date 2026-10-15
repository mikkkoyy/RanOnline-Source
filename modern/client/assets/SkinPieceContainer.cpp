// ASURA-CHARACTER-001B: the RAN `.cps` SkinPiece container reader.
//
// See SkinPieceContainer.h for the container layout and the scope statement.
// The implementation notes worth keeping next to the code:
//
//   - The cursor is the only way bytes are read. `ReadString`/`ReadU32` refuse
//     to move past the end of the buffer, so a truncated or hostile file
//     produces an error rather than a read into adjacent memory.
//
//   - Strings are length-prefixed with the count INCLUDING the trailing NUL.
//     A count of 1 is a valid empty string. A count of 0, or one that runs
//     past the end, is refused: a zero-length "string" cannot be NUL
//     terminated, so honouring it would hand the caller a non-terminated
//     buffer.
//
//   - The mesh, skeleton and node filenames are required to be non-empty.
//     The legacy loader would call `SetSkinMesh()` with whatever it read and
//     fail later; refusing here names the problem at the field that caused it.

#include "assets/SkinPieceContainer.h"

#include <cstring>

namespace Modern::Client
{

namespace
{
    // A bounds-checked forward-only reader over the container bytes.
    class Cursor
    {
    public:
        Cursor(const uint8_t* data, size_t size) : m_data(data), m_size(size) {}

        size_t Position() const noexcept { return m_pos; }
        size_t Remaining() const noexcept { return m_size - m_pos; }

        bool CanRead(size_t count) const noexcept { return m_size - m_pos >= count; }

        bool ReadU32(uint32_t& out)
        {
            if (!CanRead(4)) { return false; }
            // Little-endian, read byte by byte so the result does not depend
            // on the host's endianness or on unaligned loads.
            out = static_cast<uint32_t>(m_data[m_pos]) |
                 (static_cast<uint32_t>(m_data[m_pos + 1]) << 8) |
                 (static_cast<uint32_t>(m_data[m_pos + 2]) << 16) |
                 (static_cast<uint32_t>(m_data[m_pos + 3]) << 24);
            m_pos += 4;
            return true;
        }

        bool ReadBytes(uint8_t* out, size_t count)
        {
            if (!CanRead(count)) { return false; }
            if (out != nullptr && count > 0) { std::memcpy(out, m_data + m_pos, count); }
            m_pos += count;
            return true;
        }

        // Length-prefixed string: uint32 byteCount (including the trailing
        // NUL) followed by exactly that many bytes.
        bool ReadString(std::string& out)
        {
            uint32_t count = 0;
            if (!ReadU32(count)) { return false; }
            // Reject before allocating: a count larger than the remaining
            // bytes can never be satisfied, and must not become a huge
            // allocation driven by file content.
            if (count == 0 || static_cast<size_t>(count) > Remaining()) { return false; }
            if (!CanRead(count)) { return false; }

            const char* text = reinterpret_cast<const char*>(m_data + m_pos);
            // Defensive: the count includes a NUL, but do not rely on the
            // file having written one correctly.
            size_t length = count;
            while (length > 0 && text[length - 1] == '\0') { --length; }
            out.assign(text, length);
            m_pos += count;
            return true;
        }

    private:
        const uint8_t* m_data;
        size_t         m_size;
        size_t         m_pos = 0;
    };

    // A single material slot count cannot exceed the number of whole uint32s
    // left in the buffer; this is the sanity bound that stops a garbage count
    // from being used as an allocation or loop bound.
    bool MaterialCountIsPlausible(uint32_t count, size_t remaining)
    {
        return static_cast<size_t>(count) <= remaining / 4;
    }
} // namespace

const char* ToString(SkinPieceType type) noexcept
{
    switch (type)
    {
    case SkinPieceType::Upper:   return "UPPER";
    case SkinPieceType::Lower:   return "LOWER";
    case SkinPieceType::Hand:    return "RHAND";
    case SkinPieceType::Foot:    return "FOOT";
    case SkinPieceType::Unknown: break;
    }
    return "UNKNOWN";
}

Result<SkinPiece> SkinPieceContainer::Decode(const ResourceData& data)
{
    if (data.GetSize() < kMinimumSize) { return Status(ErrorCode::InvalidArgument); }

    const uint8_t* bytes = data.GetData();
    const size_t   size  = data.GetSize();

    Cursor cursor(bytes + kHeaderSize, size - kHeaderSize);

    uint32_t version = 0;
    if (!cursor.ReadU32(version)) { return Status(ErrorCode::InvalidArgument); }
    if (!IsSupportedVersion(version))
    {
        // An unsupported container is a distinct outcome from a malformed one:
        // the bytes may be perfectly valid RAN data this reader simply does
        // not claim to understand.
        return Status(ErrorCode::NotAllowed);
    }

    SkinPiece piece;
    piece.version = version;

    // The 128-byte container tag and the 4-byte file id are read directly:
    // they are fixed-width and precede the payload.
    {
        const char* tag = reinterpret_cast<const char*>(bytes);
        size_t      len = 0;
        while (len < kFileTypeSize && tag[len] != '\0') { ++len; }
        piece.fileType.assign(tag, len);
    }
    piece.fileId = static_cast<uint32_t>(bytes[kFileTypeSize]) |
                   (static_cast<uint32_t>(bytes[kFileTypeSize + 1]) << 8) |
                   (static_cast<uint32_t>(bytes[kFileTypeSize + 2]) << 16) |
                   (static_cast<uint32_t>(bytes[kFileTypeSize + 3]) << 24);

    uint32_t reference = 0;
    if (!cursor.ReadU32(reference)) { return Status(ErrorCode::InvalidArgument); }
    piece.reference = reference;

    if (!cursor.ReadString(piece.meshFilename))     { return Status(ErrorCode::InvalidArgument); }
    if (!cursor.ReadString(piece.skeletonFilename)) { return Status(ErrorCode::InvalidArgument); }
    if (!cursor.ReadU32(piece.pieceTypeRaw))        { return Status(ErrorCode::InvalidArgument); }
    if (!cursor.ReadU32(piece.weaponWhereBack))     { return Status(ErrorCode::InvalidArgument); }
    if (!cursor.ReadString(piece.meshNodeName))     { return Status(ErrorCode::InvalidArgument); }
    if (!cursor.ReadU32(piece.materialSlotCount))  { return Status(ErrorCode::InvalidArgument); }

    if (piece.meshFilename.empty() || piece.skeletonFilename.empty() ||
        piece.meshNodeName.empty())
    {
        return Status(ErrorCode::InvalidArgument);
    }

    if (!MaterialCountIsPlausible(piece.materialSlotCount, cursor.Remaining()))
    {
        return Status(ErrorCode::InvalidArgument);
    }

    // The material records, trace array, effect array and trailing flag are
    // deliberately left undecoded; see the scope note in the header. What
    // follows the count is captured whole so a caller can inspect or forward
    // it, and so a truncation later in the file is still visible as "the tail
    // is short" rather than silently vanishing.
    piece.opaqueTailOffset = kHeaderSize + cursor.Position();
    if (cursor.Remaining() > 0)
    {
        const uint8_t* tail = bytes + kHeaderSize + cursor.Position();
        piece.opaqueTail.assign(tail, tail + cursor.Remaining());
    }

    return Result<SkinPiece>(std::move(piece));
}

} // namespace Modern::Client