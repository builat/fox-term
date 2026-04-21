#include <Arduino.h>
#include "screen/Screen.h"
#include "keyboard/Keyboard.h"
#include "views/app/App.h"
#include "uart/UartLink.h"
#include "storage/SdStorage.h"
#include "transport/LineProcessor.h"
#include "protocol/MessageAssembler.h"
#include "Config.h"

// ---------------------------------------------------------------------------
// Global singletons
//
// All major subsystems are allocated as globals so their lifetimes match the
// microcontroller's runtime.  References to them are injected into App and
// LineProcessor rather than accessed as raw globals, which keeps the
// subsystem code independently testable.
// ---------------------------------------------------------------------------

Screen screen;     // TFT display wrapper
Keyboard keyboard; // I2C keyboard reader
UartLink uart;     // Serial1 link to the Heltec LoRa module
SdStorage storage; // SD card persistence layer

// Stateful reassembly buffer for multi-chunk incoming messages.
// A single global instance is fine because the radio is half-duplex and
// messages arrive one at a time.
MessageAssembler incomingAssembler;

// App owns all view instances and the shared DeviceState.
App app(screen, uart, storage);

// LineProcessor dispatches each UART line to the correct handler and writes
// results back into app.getState().
LineProcessor lineProcessor(app, incomingAssembler);

// Timestamp of the last periodic status request sent to the radio module.
unsigned long lastStatusRequestMs = 0;

void setup()
{
  // Debug output on the native USB serial port (not used for radio comms).
  Serial.begin(115200);

  // Serial1 talks to the Heltec module over a hardware UART at 115200 baud.
  uart.begin(115200);

  keyboard.begin();
  screen.begin();

  screen.showBootScreenStep("Initializing application...");

  // Try to mount the SD card.  If it is present we create the required
  // folder structure and load saved config; if not, we continue with the
  // compiled-in defaults.
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

  // app.begin() loads config from SD (if available) and shows the home view.
  app.begin();
  delay(1000);

  // Ask the radio module for its current status so the status bar is
  // populated before the user starts interacting.
  uart.requestStatus();
}

void loop()
{
  // 1. Read one key event per iteration (returns KEY_NONE when idle).
  KeyEvent key = keyboard.read();
  app.handleKey(key);

  // 2. Periodically poll the radio module so the RSSI/SNR values and
  //    connection status stay up to date even when no messages arrive.
  if (millis() - lastStatusRequestMs >= STATUS_INTERVAL_MS)
  {
    lastStatusRequestMs = millis();
    uart.requestStatus();
  }

  // 3. Drain every complete line that arrived on the UART since the last
  //    iteration.  uart.poll() moves raw bytes into an internal line buffer;
  //    hasLine()/popLine() expose completed lines one at a time.
  while (true)
  {
    uart.poll();

    if (!uart.hasLine())
      break;

    lineProcessor.process(uart.popLine());
  }

  // Small yield delay keeps the loop from busy-spinning too hard while still
  // feeling responsive to key presses.
  delay(30);
}