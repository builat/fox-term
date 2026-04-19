#pragma once
#include <Arduino.h>
#include "models/Messages.h"

struct IncomingMessage
{
    String id;
    String from;
    String to;
    TargetType targetType = TARGET_DEVICE;

    bool useEncryption = false;
    bool includeGps = false;

    String text;
    bool valid = true;
};
