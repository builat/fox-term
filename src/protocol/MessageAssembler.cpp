#include "protocol/MessageAssembler.h"

void MessageAssembler::reset()
{
    currentId = "";
    from = "";
    to = "";
    targetType = TARGET_DEVICE;
    includeGps = false;
    useEncryption = false;

    for (int i = 0; i < MAX_TEXT_PARTS; i++)
    {
        textParts[i] = "";
        textPartReceived[i] = false;
    }

    expectedTextParts = 0;
    gotHdr = false;
    gotGps = false;
    gotEnc = false;
    gotEnd = false;
}

void MessageAssembler::parseHeader(const String &payload)
{
    int fromPos = payload.indexOf("from=");
    int toPos = payload.indexOf("to=");
    int modePos = payload.indexOf("mode=");

    if (fromPos >= 0)
    {
        int end = payload.indexOf(';', fromPos);
        if (end < 0)
            end = payload.length();
        from = payload.substring(fromPos + 5, end);
    }

    if (toPos >= 0)
    {
        int end = payload.indexOf(';', toPos);
        if (end < 0)
            end = payload.length();
        to = payload.substring(toPos + 3, end);
    }

    if (modePos >= 0)
    {
        int end = payload.indexOf(';', modePos);
        if (end < 0)
            end = payload.length();
        String mode = payload.substring(modePos + 5, end);
        targetType = (mode == "group") ? TARGET_GROUP : TARGET_DEVICE;
    }
}

bool MessageAssembler::addChunk(const MessageChunk &chunk)
{
    if (!chunk.valid)
        return false;

    if (currentId.length() == 0)
    {
        currentId = chunk.messageId;
    }
    else if (chunk.messageId != currentId)
    {
        reset();
        currentId = chunk.messageId;
    }

    switch (chunk.type)
    {
    case CHUNK_HDR:
        parseHeader(chunk.payload);
        gotHdr = true;
        break;

    case CHUNK_GPS:
        includeGps = (chunk.payload == "1");
        gotGps = true;
        break;

    case CHUNK_ENC:
        useEncryption = (chunk.payload == "1");
        gotEnc = true;
        break;

    case CHUNK_TXT:
    {
        int slash = chunk.meta.indexOf('/');
        if (slash < 0)
            return false;

        int idx = chunk.meta.substring(0, slash).toInt() - 1;
        int total = chunk.meta.substring(slash + 1).toInt();

        if (idx < 0 || idx >= MAX_TEXT_PARTS)
            return false;
        if (total <= 0 || total > MAX_TEXT_PARTS)
            return false;

        expectedTextParts = total;
        textParts[idx] = chunk.payload;
        textPartReceived[idx] = true;
        break;
    }

    case CHUNK_END:
        gotEnd = true;
        break;

    default:
        return false;
    }

    return true;
}

bool MessageAssembler::isComplete() const
{
    if (!gotHdr || !gotGps || !gotEnc || !gotEnd)
        return false;

    if (expectedTextParts <= 0)
        return false;

    for (int i = 0; i < expectedTextParts; i++)
    {
        if (!textPartReceived[i])
            return false;
    }

    return true;
}

TransportMessage MessageAssembler::buildMessage() const
{
    TransportMessage msg;
    msg.id = currentId;
    msg.from = from;
    msg.to = to;
    msg.targetType = targetType;
    msg.includeGps = includeGps;
    msg.useEncryption = useEncryption;

    for (int i = 0; i < expectedTextParts; i++)
    {
        msg.text += textParts[i];
    }

    msg.valid = isComplete() &&
                msg.id.length() > 0 &&
                msg.from.length() > 0 &&
                msg.to.length() > 0 &&
                msg.text.length() > 0;

    return msg;
}