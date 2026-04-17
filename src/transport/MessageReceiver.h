#pragma once
#include <Arduino.h>
#include "models/IncomingMessage.h"
#include "models/Models.h"

class MessageReceiver
{
public:
    static bool isForThisDevice(const IncomingMessage &msg, const DeviceState &state);
    static bool storeToInbox(const IncomingMessage &msg, DeviceState &state);
};