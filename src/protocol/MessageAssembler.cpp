#include "protocol/MessageAssembler.h"

// ---------------------------------------------------------------------------
// MessageAssembler
//
// Stateful machine that collects MSG chunks belonging to the same message ID
// and reconstructs the original TransportMessage once all pieces have arrived.
//
// State flags track which chunk types have been received:
//   gotHdr, gotGps, gotEnc, gotEnd  — one-shot chunks (expected exactly once)
//   textPartReceived[]              — per-index flags for TXT chunks
//
// The assembler accepts chunks arriving in any order.  If a chunk for a
// different message ID arrives mid-stream, the current state is discarded
// and the new message takes over (handles the case where a previous
// transmission was partially lost).
// ---------------------------------------------------------------------------

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

// Parse the semicolon-separated key=value list carried by the HDR chunk.
// Example payload: "from=alice;to=bob;mode=group"
void MessageAssembler::parseHeader(const String &payload)
{
    int fromPos = payload.indexOf("from=");
    int toPos = payload.indexOf("to=");
    int modePos = payload.indexOf("mode=");

    // For each key, find the end of its value (next semicolon or end of string).
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

    // First chunk seen: record the message ID we are assembling.
    if (currentId.length() == 0)
    {
        currentId = chunk.messageId;
    }
    // Different ID: a new message started before the previous one completed.
    // Discard the partial state and start fresh with the new ID.
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
        // Payload is "1" (include GPS tag) or "0" (omit).
        includeGps = (chunk.payload == "1");
        gotGps = true;
        break;

    case CHUNK_ENC:
        // Payload is "1" (encrypted) or "0" (plaintext).
        useEncryption = (chunk.payload == "1");
        gotEnc = true;
        break;

    case CHUNK_TXT:
    {
        // Meta field is "<zero-based-index>/<total>", e.g. "0/3".
        // We store each part by its index so they can be reassembled in order
        // regardless of the arrival sequence.
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

// Returns true only when every expected chunk has been received.
bool MessageAssembler::isComplete() const
{
    // All four mandatory one-shot chunks must be present.
    if (!gotHdr || !gotGps || !gotEnc || !gotEnd)
        return false;

    // At least one TXT chunk must have been announced.
    if (expectedTextParts <= 0)
        return false;

    // Every individual TXT slot must be filled (no gaps).
    for (int i = 0; i < expectedTextParts; i++)
    {
        if (!textPartReceived[i])
            return false;
    }

    return true;
}

// Concatenate all TXT parts in index order and assemble the final message.
// Should only be called after isComplete() returns true.
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
        msg.text += textParts[i];

    // Mark the message valid only if all required fields are non-empty.
    msg.valid = isComplete() &&
                msg.id.length() > 0 &&
                msg.from.length() > 0 &&
                msg.to.length() > 0 &&
                msg.text.length() > 0;

    return msg;
}
