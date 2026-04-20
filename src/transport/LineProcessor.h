#pragma once
#include <Arduino.h>
#include "protocol/MessageAssembler.h"

class App;

class LineProcessor
{
public:
    LineProcessor(App &app, MessageAssembler &assembler);

    // Process a single line received from the radio UART.
    void process(const String &line);

private:
    App &app;
    MessageAssembler &assembler;

    void handleMsg(const String &line);
    void handleStat(const String &line);

    // Extract the value of key (e.g. "RSSI:") from a comma-separated status string.
    static String parseStatField(const String &line, const String &key, int keyLen);
};
