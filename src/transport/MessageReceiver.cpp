#include "models/TransportMessage.h"
#include "models/Models.h"
#include "transport/MessageReceiver.h"

bool MessageReceiver::isForThisDevice(const TransportMessage &msg, DeviceState &state)
{
    if (!msg.valid)
        return false;
    if (msg.to == state.deviceName)
        return true;
    if (msg.to == state.groupName)
        return true;
    return false;
}

void MessageReceiver::storeToInbox(const TransportMessage &msg, DeviceState &state)
{
    if (state.messageCount >= MAX_MESSAGES)
    {
        state.lastTransportStatus = "Inbox full";
        return;
    }

    Message &m = state.messages[state.messageCount];
    m.id = msg.id;
    m.from = msg.from;
    m.to = msg.to;
    m.title = "[ " + msg.from + " ] [ " + msg.id + " ]";
    m.body = msg.text;
    m.unread = true;
    state.messageCount++;
}