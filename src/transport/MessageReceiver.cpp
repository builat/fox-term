#include "models/TransportMessage.h"
#include "models/Models.h"
#include "transport/MessageReceiver.h"

// Returns true if this device should keep the message.
// A message is accepted if it is addressed to the device's individual name
// (unicast) or to the group it belongs to (broadcast).
bool MessageReceiver::isForThisDevice(const TransportMessage &msg, DeviceState &state)
{
    return msg.valid && (msg.to == state.deviceName || msg.to == state.groupName);
}

// Appends the message to the in-memory inbox array inside DeviceState.
// The inbox has a fixed capacity (MAX_MESSAGES); once full, new messages are
// dropped and the transport status is updated so the user can see it.
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
    // The title is a display string shown in the message list view.
    m.title = "[ " + msg.from + " ] [ " + msg.id + " ]";
    m.body = msg.text;
    m.unread = true;
    state.messageCount++;
}
