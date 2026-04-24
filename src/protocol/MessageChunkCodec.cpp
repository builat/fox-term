#include "protocol/MessageChunkCodec.h"

// ---------------------------------------------------------------------------
// Wire format overview
//
// Every line on the UART is pipe-delimited with exactly 5 fields:
//
//   MSG | <message-id> | <chunk-type> | <meta> | <payload>
//
// Example sequence for a two-part text message:
//   MSG|1234-1|HDR||from=alice;to=bob;mode=device
//   MSG|1234-1|GPS||0
//   MSG|1234-1|ENC||0
//   MSG|1234-1|TXT|0/2|Hello, th
//   MSG|1234-1|TXT|1/2|is is it.
//   MSG|1234-1|END||
//
// The meta field is only used by TXT chunks where it carries the
// zero-based chunk index and total count as "<idx>/<total>".
// ---------------------------------------------------------------------------

String MessageChunkCodec::targetTypeToString(TargetType t)
{
    return (t == TARGET_GROUP) ? "group" : "device";
}

ChunkType MessageChunkCodec::parseChunkType(const String &s)
{
    if (s == "HDR")
        return CHUNK_HDR;
    if (s == "GPS")
        return CHUNK_GPS;
    if (s == "ENC")
        return CHUNK_ENC;
    if (s == "TXT")
        return CHUNK_TXT;
    if (s == "END")
        return CHUNK_END;
    return CHUNK_UNKNOWN;
}

// Removes characters that would corrupt the pipe-delimited wire format or
// introduce spurious line breaks.
String MessageChunkCodec::sanitize(const String &s)
{
    String out = s;
    out.replace("|", "/");  // pipes are field separators
    out.replace("\n", " "); // newlines would be misread as line endings
    out.replace("\r", " ");
    return out;
}

// ---------------------------------------------------------------------------
// Encode  —  TransportMessage  →  array of UART lines
//
// Produces the fixed-order HDR / GPS / ENC / TXT… / END sequence.
// Text longer than TEXT_CHUNK_SIZE bytes is split into consecutive TXT
// chunks.  The caller must provide a buffer large enough for all of them
// (minimum 5 slots: HDR + GPS + ENC + at least one TXT + END).
// Returns the number of lines written, or 0 if maxChunks is too small.
// ---------------------------------------------------------------------------

int MessageChunkCodec::encode(const TransportMessage &msg, String outChunks[], int maxChunks)
{
    if (maxChunks < 5)
        return 0;

    int index = 0;

    // Sanitize user-supplied strings before embedding them in the wire format.
    String from = sanitize(msg.from);
    String to = sanitize(msg.to);
    String text = sanitize(msg.text);

    // HDR carries the routing fields (sender, recipient, unicast vs. group).
    outChunks[index++] =
        "MSG|" + msg.id + "|HDR||from=" + from +
        ";to=" + to +
        ";mode=" + targetTypeToString(msg.targetType);

    // GPS and ENC are single-bit flags packed as "1" or "0".
    outChunks[index++] = "MSG|" + msg.id + "|GPS||" + String(msg.includeGps ? "1" : "0");
    outChunks[index++] = "MSG|" + msg.id + "|ENC||" + String(msg.useEncryption ? "1" : "0");

    // Split text into fixed-size chunks.  At least one TXT chunk is always
    // emitted even for an empty body so the receiver can reconstruct the
    // message without special-casing zero-length text.
    int totalChunks = (text.length() + TEXT_CHUNK_SIZE - 1) / TEXT_CHUNK_SIZE;
    if (totalChunks == 0)
        totalChunks = 1;

    for (int i = 0; i < totalChunks; i++)
    {
        // Stop early if we would overflow the buffer (leave one slot for END).
        if (index >= maxChunks - 1)
            return index;

        int start = i * TEXT_CHUNK_SIZE;
        String part = text.substring(start, start + TEXT_CHUNK_SIZE);

        outChunks[index++] =
            "MSG|" + msg.id + "|TXT|" + String(i + 1) + "/" + String(totalChunks) + "|" + part;
    }

    // END marks the final boundary of a transmission for this message ID.
    if (index < maxChunks)
        outChunks[index++] = "MSG|" + msg.id + "|END||";

    return index;
}

// ---------------------------------------------------------------------------
// Decode  —  raw UART line  →  MessageChunk
//
// Locates the four pipe separators by scanning left to right; substrings
// between them become the struct fields.  Returns an invalid chunk if fewer
// than four pipes are found or the prefix is wrong.
// ---------------------------------------------------------------------------

MessageChunk MessageChunkCodec::decode(const String &line)
{
    MessageChunk chunk; // chunk.valid defaults to false

    if (!line.startsWith("MSG|"))
        return chunk;

    // Locate all four field separators in one pass.
    int p1 = line.indexOf('|');         // after "MSG"
    int p2 = line.indexOf('|', p1 + 1); // after message-id
    int p3 = line.indexOf('|', p2 + 1); // after chunk-type
    int p4 = line.indexOf('|', p3 + 1); // after meta

    if (p1 < 0 || p2 < 0 || p3 < 0 || p4 < 0)
        return chunk;

    chunk.messageId = line.substring(p1 + 1, p2);
    chunk.type = parseChunkType(line.substring(p2 + 1, p3));
    chunk.meta = line.substring(p3 + 1, p4);
    chunk.payload = line.substring(p4 + 1);

    // A chunk is considered valid if we have a non-empty id OR a recognised
    // type (END chunks intentionally carry an empty id in some edge cases).
    chunk.valid = chunk.messageId.length() > 0 || chunk.type != CHUNK_UNKNOWN;

    return chunk;
}
