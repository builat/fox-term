#include <Arduino.h>
#include "screen/Screen.h"
#include "keyboard/Keyboard.h"
#include "views/app/App.h"
#include "uart/UartLink.h"
#include <SD.h>
#include "transport/MessageDispatcher.h"
#include "transport/MessageReceiver.h"
#include "protocol/MessageChunkCodec.h"
#include "protocol/MessageParser.h"
#include "protocol/MessageAssembler.h"
#include "models/TransportMessage.h"

Screen screen;
Keyboard keyboard;
UartLink uart;
MessageReceiver receiver;
MessageAssembler incomingAssembler;

App app(screen, uart);

unsigned long lastStatusRequestMs = 0;
const unsigned long STATUS_INTERVAL_MS = 60 * 1000;

void processIncomingLine(const String &line)
{
  DeviceState &state = app.getState();

  // Serial.println("[UART] " + line);
  if (line == "")
  {
    return;
  }

  if (line.startsWith("MSG|"))
  {
    MessageChunk chunk = MessageChunkCodec::decode(line);

    if (!chunk.valid)
    {
      state.lastTransportStatus = "Bad RX chunk";
      return;
    }

    if (!incomingAssembler.addChunk(chunk))
    {
      state.lastTransportStatus = "RX assembly failed";
      incomingAssembler.reset();
      return;
    }

    if (incomingAssembler.isComplete())
    {
      TransportMessage msg = incomingAssembler.buildMessage();
      incomingAssembler.reset();

      if (!msg.valid)
      {
        state.lastTransportStatus = "RX invalid message";
        return;
      }
      if (MessageReceiver::isForThisDevice(msg, state))
      {
        MessageReceiver::storeToInbox(msg, state);
        state.lastTransportStatus = "RX stored: " + msg.id;
      }
      else
      {
        state.lastTransportStatus = "RX skipped";
      }
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
    app.setHeltecConnected(true);

    int rssiPos = line.indexOf("RSSI:");
    int snrPos = line.indexOf("SNR:");

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
    // Serial.println(chipSelect);

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
  delay(1000);

  uart.requestStatus();
}

void loop()
{
  KeyEvent key = keyboard.read();
  app.handleKey(key);

  if (millis() - lastStatusRequestMs >= STATUS_INTERVAL_MS)
  {
    lastStatusRequestMs = millis();
    uart.requestStatus();
  }

  while (true)
  {
    uart.poll();

    if (!uart.hasLine())
      break;

    processIncomingLine(uart.popLine());
  }

  delay(30);
}