#include "uart/UartLink.h"

void UartLink::begin(unsigned long baud)
{
    Serial1.begin(baud);
}

void UartLink::poll()
{
    while (Serial1.available())
    {
        char c = (char)Serial1.read();
        lastSeenMs = millis();

        if (c == '\r')
        {
            continue;
        }

        if (c == '\n')
        {
            if (rxBuffer.length() > 0)
            {
                readyLine = rxBuffer;
                rxBuffer = "";
                lineReady = true;
            }
        }
        else
        {
            rxBuffer += c;

            if (rxBuffer.length() > 180)
            {
                rxBuffer = "";
            }
        }
    }
}

void UartLink::sendLine(const String &line)
{
    Serial1.println(line);
}

void UartLink::sendPing()
{
    sendLine("PING");
}

void UartLink::sendMessage(const String &body)
{
    sendLine("SEND:" + body);
}

bool UartLink::hasLine() const
{
    return lineReady;
}

String UartLink::popLine()
{
    lineReady = false;
    return readyLine;
}

bool UartLink::isConnected() const
{
    return (millis() - lastSeenMs) < TIMEOUT_MS;
}

unsigned long UartLink::getLastSeenMs() const
{
    return lastSeenMs;
}

void UartLink::requestStatus()
{
    sendLine("STAT");
}