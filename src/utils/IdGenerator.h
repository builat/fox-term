#pragma once
#include <Arduino.h>

class IdGenerator
{
public:
    static String makeMessageId(const String &deviceName, int seq)
    {
        String out = deviceName;
        out += "-";
        out += String((unsigned long)millis());
        out += "-";
        out += String(seq);
        return out;
    }
};