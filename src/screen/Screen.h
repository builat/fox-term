#pragma once

#include <Arduino.h>
#include <DIYables_TFT_Shield.h>

class Screen
{
public:
  void begin();

  void clear();
  void newLine();

  void printText(const String &text, uint16_t color);
  void printLine(const String &text, uint16_t color);
  void printDefaultColorLine(const String &text);
  void showBootScreenStep(const String &line);
  void appendBootLog(const String &line);
  void printPrompt();

  void printChar(char c, uint16_t color);
  void eraseCharCell();

  void updateCursor();
  void resetCursorBlink();

  void setRadioStatus(bool connected, const String &rssi, const String &snr);
  void renderStatusBar();

  int getCursorX() const;
  int getCursorY() const;
  void setCursorX(int x);
  void setCursorY(int y);
  void eraseCursor();
  void drawCursor();

private:
  DIYables_TFT_ILI9486_Shield tft;

  int cursorX;
  int cursorY;
  bool cursorVisible;
  unsigned long lastCursorToggle;
  unsigned long lastStatusUpdate;
  bool lastHeltecConnected;
  bool heltecConnected;
};