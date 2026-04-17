#include <Arduino.h>
#include "screen/Screen.h"
#include "keyboard/Keyboard.h"
#include "views/app/App.h"
#include "uart/UartLink.h"
#include <SD.h>
#include "protocol/MessageParser.h"
#include "transport/MessageReceiver.h"

Screen screen;
Keyboard keyboard;
UartLink uart;
App app(screen, uart);

unsigned long lastStatusRequestMs = 0;
const unsigned long STATUS_INTERVAL_MS = 5000;

void processIncomingLine(const String &line)
{
  DeviceState &state = app.getState();

  Serial.println("[UART] " + line);

  if (line.startsWith("RX:"))
  {
    String payload = line.substring(3);
    IncomingMessage msg = MessageParser::parse(payload);

    if (!msg.valid)
    {
      state.lastTransportStatus = "RX parse failed";
      return;
    }

    if (MessageReceiver::isForThisDevice(msg, state))
    {
      bool stored = MessageReceiver::storeToInbox(msg, state);
      state.lastTransportStatus = stored ? "RX stored: " + msg.id
                                         : "RX inbox full";
    }
    else
    {
      state.lastTransportStatus = "RX skipped: not for me";
    }

    return;
  }

  if (line == "OK:SEND")
  {
    state.lastTransportStatus = "Send OK";
    return;
  }

  if (line.startsWith("ERR:"))
  {
    state.lastTransportStatus = line;
    return;
  }

  if (line.startsWith("STAT:"))
  {

    int rssiPos = line.indexOf("RSSI:");
    int snrPos = line.indexOf("SNR:");

    app.setRadioStatus(true, String(rssiPos), String(snrPos));

    if (rssiPos >= 0)
    {
      int comma = line.indexOf(',', rssiPos);
      if (comma < 0)
        comma = line.length();
      state.lastRssi = line.substring(rssiPos + 5, comma);
    }

    if (snrPos >= 0)
    {
      int comma = line.indexOf(',', snrPos);
      if (comma < 0)
        comma = line.length();
      state.lastSnr = line.substring(snrPos + 4, comma);
    }

    return;
  }
}

void setup()
{
  Serial.begin(115200);

  keyboard.begin();
  uart.begin(115200);
  screen.begin();
  screen.showBootScreenStep("Initializing application...");
  const int chipSelect = 53; // Change this to your desired pin

  if (SD.begin(chipSelect))
  {
    Serial.print("SD init OK on CS ");
    Serial.println(chipSelect);

    File f = SD.open("/test.txt", FILE_WRITE);
    if (f)
    {
      f.println("hello");
      f.close();
      screen.printLine("SD card initialized successfully.", 0x00FF00U);
    }
    else
    {
      screen.printLine("SD card initialization failed.", 0xFF0000U);
    }
  }
  else
  {
    screen.printLine("SD card initialization failed.", 0xFF0000U);
  }

  delay(2000);
  app.begin();
}

void loop()
{
  KeyEvent key = keyboard.read();
  app.handleKey(key);

  uart.poll();

  while (uart.hasLine())
  {
    processIncomingLine(uart.popLine());
  }

  if (millis() - lastStatusRequestMs >= STATUS_INTERVAL_MS)
  {
    Serial.println("[APP] Requesting status update...");
    lastStatusRequestMs = millis();
    uart.requestStatus();
  }
  delay(30);
}