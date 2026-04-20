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
// ---------------------------------------------------------------------------

void LineProcessor::process(const String &line)
{
    if (line.length() == 0)
        return;

    if (line.startsWith("MSG|"))  { handleMsg(line);  return; }
    if (line.startsWith("STAT:")) { handleStat(line); return; }
    if (line.startsWith("ERR:"))  { app.getState().lastTransportStatus = line; return; }
    if (line == "OK:SEND")        { app.getState().lastTransportStatus = "Send OK"; return; }
}

// ---------------------------------------------------------------------------
// MSG chunk handling
// ---------------------------------------------------------------------------

void LineProcessor::handleMsg(const String &line)
{
    DeviceState &state = app.getState();

    MessageChunk chunk = MessageChunkCodec::decode(line);
    if (!chunk.valid)
    {
        state.lastTransportStatus = "Bad RX chunk";
        return;
    }

    if (!assembler.addChunk(chunk))
    {
        state.lastTransportStatus = "RX assembly failed";
        assembler.reset();
        return;
    }

    if (!assembler.isComplete())
        return;

    TransportMessage msg = assembler.buildMessage();
    assembler.reset();

    if (!msg.valid)
    {
        state.lastTransportStatus = "RX invalid message";
        return;
    }

    if (!MessageReceiver::isForThisDevice(msg, state))
    {
        state.lastTransportStatus = "RX skipped";
        return;
    }

    MessageReceiver::storeToInbox(msg, state);
    state.lastTransportStatus = "RX stored: " + msg.id;
}

// ---------------------------------------------------------------------------
// STAT line handling  (e.g. "STAT:RSSI:-42,SNR:9.2")
// ---------------------------------------------------------------------------

String LineProcessor::parseStatField(const String &line, const String &key, int keyLen)
{
    int pos = line.indexOf(key);
    if (pos < 0)
        return "";

    int start = pos + keyLen;
    int comma = line.indexOf(',', start);
    return line.substring(start, comma < 0 ? line.length() : comma);
}

void LineProcessor::handleStat(const String &line)
{
    app.setHeltecConnected(true);

    DeviceState &state = app.getState();
    state.lastRssi = parseStatField(line, "RSSI:", 5);
    state.lastSnr  = parseStatField(line, "SNR:",  4);
}
