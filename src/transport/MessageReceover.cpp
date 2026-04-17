#include "transport/MessageReceiver.h"

bool MessageReceiver::isForThisDevice(const IncomingMessage &msg, const DeviceState &state)
{
    if (!msg.valid)
        return false;

    if (msg.to == state.deviceName)
        return true;

    if (msg.to == state.groupName)
        return true;

    return false;
}

bool MessageReceiver::storeToInbox(const IncomingMessage &msg, DeviceState &state)
{
    if (state.messageCount >= MAX_MESSAGES)
        return false;

    Message &m = state.messages[state.messageCount];
    m.id = msg.id;
    m.from = msg.from;
    m.to = msg.to;
    m.title = "Incoming";
    m.body = msg.text;
    m.unread = true;

    state.messageCount++;
    return true;
}