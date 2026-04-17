#pragma once
#include <Arduino.h>

class UartLink
{
public:
    void begin(unsigned long baud = 115200);
    void poll();

    void sendLine(const String &line);
    void sendPing();
    void sendMessage(const String &body);
    void requestStatus();

    bool hasLine() const;
    String popLine();

    bool isConnected() const;
    unsigned long getLastSeenMs() const;

private:
    String rxBuffer;
    String readyLine;
    bool lineReady = false;
    unsigned long lastSeenMs = 0;
    static const unsigned long TIMEOUT_MS = 5000;
};