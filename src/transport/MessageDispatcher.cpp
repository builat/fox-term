#include "transport/MessageDispatcher.h"
#include "uart/UartLink.h"
#include "protocol/MessageChunkCodec.h"

MessageDispatcher::MessageDispatcher(UartLink &u) : uart(u)
{
}

bool MessageDispatcher::dispatch(const TransportMessage &msg)
{
    String chunks[32];
    int count = MessageChunkCodec::encode(msg, chunks, 32);
    if (count <= 0)
        return false;

    for (int i = 0; i < count; i++)
    {
        uart.sendLine(chunks[i]);
    }

    return true;
}