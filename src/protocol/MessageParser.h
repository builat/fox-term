#pragma once
#include <Arduino.h>
#include "models/IncomingMessage.h"

class MessageParser
{
public:
    static IncomingMessage parse(const String &payload);

private:
    static String getField(const String &payload, const String &key);
};