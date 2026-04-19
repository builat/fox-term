#pragma once
#include <Arduino.h>
#include "models/TransportMessage.h"
#include "models/Messages.h"

class MessageAssembler
{
public:
    void reset();
    bool addChunk(const MessageChunk &chunk);
    bool isComplete() const;
    TransportMessage buildMessage() const;

private:
    String currentId;
    String from;
    String to;
    TargetType targetType = TARGET_DEVICE;
    bool includeGps = false;
    bool useEncryption = false;

    static const int MAX_TEXT_PARTS = 16;
    String textParts[MAX_TEXT_PARTS];
    bool textPartReceived[MAX_TEXT_PARTS] = {false};
    int expectedTextParts = 0;

    bool gotHdr = false;
    bool gotGps = false;
    bool gotEnc = false;
    bool gotEnd = false;

    void parseHeader(const String &payload);
};