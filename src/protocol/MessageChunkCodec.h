#pragma once
#include <Arduino.h>
#include "models/TransportMessage.h"
#include "models/Messages.h"

class MessageChunkCodec
{
public:
    static const size_t TEXT_CHUNK_SIZE = 32;

    static int encode(const TransportMessage &msg, String outChunks[], int maxChunks);
    static MessageChunk decode(const String &line);

private:
    static String targetTypeToString(TargetType t);
    static ChunkType parseChunkType(const String &s);
    static String sanitize(const String &s);
};