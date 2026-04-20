#include <Arduino.h>
#include "screen/Screen.h"
#include "keyboard/Keyboard.h"
#include "views/app/App.h"
#include "uart/UartLink.h"
#include "storage/SdStorage.h"
#include "transport/LineProcessor.h"
#include "protocol/MessageAssembler.h"
#include "Config.h"

Screen screen;
Keyboard keyboard;
UartLink uart;
SdStorage storage;
MessageAssembler incomingAssembler;

App app(screen, uart, storage);
LineProcessor lineProcessor(app, incomingAssembler);

unsigned long lastStatusRequestMs = 0;

void setup()
{
  Serial.begin(115200);
  uart.begin(115200);

  keyboard.begin();
  screen.begin();
  
  screen.showBootScreenStep("Initializing application...");

  if (storage.begin(SD_CS_PIN))
  {
    storage.ensureFileStructure();
    screen.printLine("SD card ready.", GREEN);
  }
  else
  {
    screen.printLine("SD card not available.", RED);
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

    lineProcessor.process(uart.popLine());
  }

  delay(30);
}