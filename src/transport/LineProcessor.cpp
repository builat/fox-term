#include "transport/LineProcessor.h"
#include "views/app/App.h"
#include "protocol/MessageChunkCodec.h"
#include "transport/MessageReceiver.h"

LineProcessor::LineProcessor(App &a, MessageAssembler &asm_)
    : app(a), assembler(asm_)
{
}

// ---------------------------------------------------------------------------
// Public entry point
//
// Acts as a simple prefix-based router.  Each protocol verb maps to a private
// handler; unknown lines are silently ignored.
// ---------------------------------------------------------------------------

void LineProcessor::process(const String &line)
{
    if (line.length() == 0)
        return;

    if (line.startsWith("MSG|"))
    {
        handleMsg(line);
        return;
    }
    if (line.startsWith("STAT:"))
    {
        handleStat(line);
        return;
    }
    if (line.startsWith("ERR:"))
    {
        app.getState().lastTransportStatus = line;
        return;
    }
    if (line == "OK:SEND")
    {
        app.getState().lastTransportStatus = "Send OK";
        return;
    }
}

// ---------------------------------------------------------------------------
// MSG chunk handling
//
// The radio protocol splits every message into named chunks so that large
// payloads fit within the LoRa packet MTU.  A complete transmission looks
// like this (one line per chunk):
//
//   MSG|<id>|HDR||from=alice;to=bob;mode=device
//   MSG|<id>|GPS||0
//   MSG|<id>|ENC||0
//   MSG|<id>|TXT|0/1|Hello!
//   MSG|<id>|END||
//
// MessageAssembler collects these in any order and signals when the full set
// has been received.  Only then is the message validated and stored.
// ---------------------------------------------------------------------------

void LineProcessor::handleMsg(const String &line)
{
    DeviceState &state = app.getState();

    // Decode the raw line into a typed MessageChunk struct.
    MessageChunk chunk = MessageChunkCodec::decode(line);
    if (!chunk.valid)
    {
        state.lastTransportStatus = "Bad RX chunk";
        return;
    }

    // Feed the chunk to the stateful assembler.  Returns false if the chunk
    // is structurally invalid (e.g. out-of-range TXT index).
    if (!assembler.addChunk(chunk))
    {
        state.lastTransportStatus = "RX assembly failed";
        assembler.reset();
        return;
    }

    // Not all chunks of this message have arrived yet — wait for more.
    if (!assembler.isComplete())
        return;

    // All chunks received: build the final message and clear the assembler
    // so it is ready for the next incoming message.
    TransportMessage msg = assembler.buildMessage();
    assembler.reset();

    if (!msg.valid)
    {
        state.lastTransportStatus = "RX invalid message";
        return;
    }

    // Check whether this device is the intended recipient (by device name or
    // group name).  Messages addressed to others are discarded.
    if (!MessageReceiver::isForThisDevice(msg, state))
    {
        state.lastTransportStatus = "RX skipped";
        return;
    }

    // Store the message in the in-memory inbox so the Messages view can
    // display it.
    MessageReceiver::storeToInbox(msg, state);
    state.lastTransportStatus = "RX stored: " + msg.id;
}

// ---------------------------------------------------------------------------
// STAT line handling
//
// The module sends a status line at regular intervals (or on demand via the
// "STAT" command).  Format: "STAT:RSSI:<dBm>,SNR:<dB>"
// Example:              "STAT:RSSI:-87,SNR:6.5"
//
// We mark the module as connected and update the signal-quality fields so
// the status bar can show them.
// ---------------------------------------------------------------------------

String LineProcessor::parseStatField(const String &line, const String &key, int keyLen)
{
    // Find where the key starts in the line.
    int pos = line.indexOf(key);
    if (pos < 0)
        return "";

    // The value begins immediately after the key and ends at the next comma
    // (or at the end of the string for the last field).
    int start = pos + keyLen;
    int comma = line.indexOf(',', start);
    return line.substring(start, comma < 0 ? line.length() : comma);
}

void LineProcessor::handleStat(const String &line)
{
    // Any STAT line means the module is reachable — update connection state.
    app.setHeltecConnected(true);

    DeviceState &state = app.getState();
    state.lastRssi = parseStatField(line, "RSSI:", 5);
    state.lastSnr = parseStatField(line, "SNR:", 4);
}
