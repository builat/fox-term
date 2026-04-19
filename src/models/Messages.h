#pragma once
#include <Arduino.h>

/*
 * DEPRICATED - see TransportMessage.h
 */

enum TargetType
{
    TARGET_DEVICE,
    TARGET_GROUP
};

enum MessageStatus
{
    MSG_DRAFT,
    MSG_QUEUED,
    MSG_SENDING,
    MSG_SENT,
    MSG_FAILED
};

inline String targetTypeToString(TargetType t)
{
    return (t == TARGET_GROUP) ? "group" : "device";
}

inline String statusToString(MessageStatus s)
{
    switch (s)
    {
    case MSG_DRAFT:
        return "draft";
    case MSG_QUEUED:
        return "queued";
    case MSG_SENDING:
        return "sending";
    case MSG_SENT:
        return "sent";
    case MSG_FAILED:
        return "failed";
    default:
        return "unknown";
    }
}

enum ChunkType
{
    CHUNK_HDR,
    CHUNK_GPS,
    CHUNK_ENC,
    CHUNK_TXT,
    CHUNK_END,
    CHUNK_UNKNOWN
};

struct MessageChunk
{
    String messageId;
    ChunkType type = CHUNK_UNKNOWN;
    String meta;
    String payload;
    bool valid = false;
};