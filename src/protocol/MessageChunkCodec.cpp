#include "protocol/MessageChunkCodec.h"

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

String MessageChunkCodec::sanitize(const String &s)
{
    String out = s;
    out.replace("|", "/");
    out.replace("\n", " ");
    out.replace("\r", " ");
    return out;
}

int MessageChunkCodec::encode(const TransportMessage &msg, String outChunks[], int maxChunks)
{
    if (maxChunks < 5)
        return 0;

    int index = 0;

    String from = sanitize(msg.from);
    String to = sanitize(msg.to);
    String text = sanitize(msg.text);

    outChunks[index++] =
        "MSG|" + msg.id + "|HDR||from=" + from +
        ";to=" + to +
        ";mode=" + targetTypeToString(msg.targetType);

    outChunks[index++] =
        "MSG|" + msg.id + "|GPS||" + String(msg.includeGps ? "1" : "0");

    outChunks[index++] =
        "MSG|" + msg.id + "|ENC||" + String(msg.useEncryption ? "1" : "0");

    int totalChunks = (text.length() + TEXT_CHUNK_SIZE - 1) / TEXT_CHUNK_SIZE;
    if (totalChunks == 0)
        totalChunks = 1;

    for (int i = 0; i < totalChunks; i++)
    {
        if (index >= maxChunks - 1)
            return index;

        int start = i * TEXT_CHUNK_SIZE;
        String part = text.substring(start, start + TEXT_CHUNK_SIZE);

        outChunks[index++] =
            "MSG|" + msg.id + "|TXT|" + String(i) + "/" + String(totalChunks) + "|" + part;
    }

    if (index < maxChunks)
    {
        outChunks[index++] = "MSG|" + msg.id + "|END||";
    }

    return index;
}

MessageChunk MessageChunkCodec::decode(const String &line)
{
    MessageChunk chunk;

    if (!line.startsWith("MSG|"))
        return chunk;

    int p1 = line.indexOf('|');
    int p2 = line.indexOf('|', p1 + 1);
    int p3 = line.indexOf('|', p2 + 1);
    int p4 = line.indexOf('|', p3 + 1);

    if (p1 < 0 || p2 < 0 || p3 < 0 || p4 < 0)
        return chunk;

    chunk.messageId = line.substring(p1 + 1, p2);

    String typeStr = line.substring(p2 + 1, p3);
    chunk.type = parseChunkType(typeStr);

    chunk.meta = line.substring(p3 + 1, p4);
    chunk.payload = line.substring(p4 + 1);
    chunk.valid = chunk.messageId.length() > 0 || chunk.type != CHUNK_UNKNOWN;

    return chunk;
}