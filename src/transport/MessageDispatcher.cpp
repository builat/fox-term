#include "transport/MessageDispatcher.h"
#include "uart/UartLink.h"

MessageDispatcher::MessageDispatcher(UartLink &u) : uart(u)
{
}

String MessageDispatcher::encode(const OutgoingMessage &msg) const
{
    String line = "SEND:";
    line += "id=" + msg.id;
    line += "|from=" + msg.from;
    line += "|to=" + msg.to;
    line += "|target=" + targetTypeToString(msg.targetType);
    line += "|enc=" + String(msg.useEncryption ? "1" : "0");
    line += "|gps=" + String(msg.includeGps ? "1" : "0");
    line += "|text=" + msg.text;

    return line;
}

bool MessageDispatcher::dispatch(OutgoingMessage &msg)
{
    msg.status = MSG_SENDING;
    uart.sendLine(encode(msg));
    return true;
}