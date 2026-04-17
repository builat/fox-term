#pragma once
#include <Arduino.h>
#include "models/Messages.h"

class UartLink;

class MessageDispatcher
{
public:
    MessageDispatcher(UartLink &uart);

    bool dispatch(OutgoingMessage &msg);
    String encode(const OutgoingMessage &msg) const;

private:
    UartLink &uart;
};