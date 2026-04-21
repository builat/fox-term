#include "uart/UartLink.h"

void UartLink::begin(unsigned long baud)
{
    // The Heltec module is wired to the Arduino Mega's Serial1 hardware UART
    // (pins 18/19).  Serial (USB) is reserved for debug output.
    Serial1.begin(baud);
}

// ---------------------------------------------------------------------------
// poll()  —  called every loop iteration to drain the hardware RX buffer
//
// Bytes are appended to rxBuffer until a newline (\n) arrives; at that point
// the complete line is latched into readyLine and lineReady is set.  The
// caller retrieves it with hasLine() / popLine().
//
// Only one line is "ready" at a time.  If a second line arrives before the
// first is consumed it will overwrite readyLine, so the caller must drain
// the buffer promptly (the main loop does this in a tight while-true).
//
// A hard cap of 180 bytes discards runaway input that never contained a
// newline (e.g. garbled packets), preventing unbounded memory growth.
// ---------------------------------------------------------------------------

void UartLink::poll()
{
    while (Serial1.available())
    {
        char c = (char)Serial1.read();

        // Update the last-seen timestamp on every received byte so that
        // isConnected() reflects recent activity even between status lines.
        lastSeenMs = millis();

        if (c == '\r')
        {
            // Ignore carriage returns; we delimit on \n only.
            continue;
        }

        if (c == '\n')
        {
            // Complete line received — latch it for the caller.
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

            // Safety cap: drop the buffer if a line grows unreasonably large.
            if (rxBuffer.length() > 180)
                rxBuffer = "";
        }
    }
}

void UartLink::sendLine(const String &line)
{
    // println() appends \r\n which the Heltec firmware expects as a
    // command terminator.
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
    // Clear the flag before returning so the next poll() can latch a new line.
    lineReady = false;
    return readyLine;
}

// Returns true if a byte was received within the last TIMEOUT_MS milliseconds.
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
    // "STAT" prompts the Heltec module to reply with a STAT: line containing
    // the current RSSI and SNR of the last received packet.
    sendLine("STAT");
}
