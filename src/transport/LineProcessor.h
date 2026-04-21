#pragma once
#include <Arduino.h>
#include "protocol/MessageAssembler.h"

class App;

/**
 * LineProcessor
 *
 * Parses and dispatches a single newline-terminated string received from the
 * Heltec radio module over UART.  The module speaks a simple text protocol:
 *
 *   MSG|<id>|<type>|<meta>|<payload>   — one chunk of an in-flight message
 *   STAT:RSSI:<val>,SNR:<val>          — periodic radio status report
 *   OK:SEND                            — acknowledgement after a send
 *   ERR:<reason>                       — error report from the module
 *
 * Multi-chunk message reassembly is delegated to a shared MessageAssembler.
 * Completed, validated messages are forwarded to MessageReceiver which
 * decides whether to store them in the in-memory inbox.
 */
class LineProcessor
{
public:
    LineProcessor(App &app, MessageAssembler &assembler);

    // Dispatch a single line received from the radio UART.
    void process(const String &line);

private:
    App &app;
    MessageAssembler &assembler;

    // Handles the multi-step MSG chunk protocol.
    void handleMsg(const String &line);

    // Parses a STAT: line and updates radio connection state.
    void handleStat(const String &line);

    // Returns the value that follows `key` up to the next comma (or end of
    // string).  keyLen must equal key.length() and is passed explicitly to
    // avoid recomputing it on every call.
    static String parseStatField(const String &line, const String &key, int keyLen);
};
