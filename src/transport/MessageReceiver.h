#pragma once
#include <Arduino.h>
#include "models/TransportMessage.h"
#include "models/Models.h"

class MessageReceiver
{
public:
    static bool isForThisDevice(const TransportMessage &msg, DeviceState &state);
    static void storeToInbox(const TransportMessage &msg, DeviceState &state);
};