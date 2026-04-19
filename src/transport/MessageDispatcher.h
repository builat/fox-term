#pragma once
#include <Arduino.h>
#include "models/TransportMessage.h"

class UartLink;

class MessageDispatcher
{
public:
    MessageDispatcher(UartLink &uart);
    bool dispatch(const TransportMessage &msg);

private:
    UartLink &uart;
};