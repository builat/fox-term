#pragma once
#include <Arduino.h>

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

struct OutgoingMessage
{
    String id;
    String from;
    String to;
    TargetType targetType = TARGET_DEVICE;

    bool useEncryption = false;
    bool includeGps = true;

    String text;
    String createdAt;
    MessageStatus status = MSG_DRAFT;
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