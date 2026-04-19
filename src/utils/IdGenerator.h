#pragma once
#include <Arduino.h>

class IdGenerator
{
public:
    static String makeMessageId(int seq)
    {
        String out = String((unsigned long)millis());
        out += "-";
        out += String(seq);
        return out;
    }
};