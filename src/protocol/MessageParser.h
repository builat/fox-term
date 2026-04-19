#pragma once
#include <Arduino.h>
#include "models/TransportMessage.h"

class MessageParser
{
public:
    static TransportMessage parse(const String &payload);

private:
    static String getField(const String &payload, const String &key);
};