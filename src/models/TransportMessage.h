#pragma once
#include <Arduino.h>
#include "models/Messages.h"

struct TransportMessage
{
    String id;
    String from;
    String to;
    TargetType targetType = TARGET_DEVICE;
    bool includeGps = false;
    bool useEncryption = false;
    String text;
    bool valid = false;
};