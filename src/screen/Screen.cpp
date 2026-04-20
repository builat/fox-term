#include "Screen.h"
#include "Config.h"

void Screen::begin()
{
  tft.begin();
  tft.setRotation(1);
  tft.setTextSize(1);

  cursorX = LEFT_MARGIN;
  cursorY = TOP_MARGIN;
  cursorVisible = false;
  lastCursorToggle = 0;
  heltecConnected = false;

  clear();
}

void Screen::renderStatusBar()
{
  tft.fillRect(0, 0, SCREEN_WIDTH, STATUS_BAR_HEIGHT, BLACK);

  tft.setTextColor(WHITE);
  tft.setCursor(2, 2);

  tft.setTextColor(heltecConnected ? GREEN : RED);
  tft.setCursor(SCREEN_WIDTH - 95, 2);
  tft.print("RADIO: ");
  tft.print(heltecConnected ? "ON" : "OFF");

  tft.drawLine(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, STATUS_BAR_HEIGHT, GREEN);
}

void Screen::setRadioStatus(bool connected, const String &rssi, const String &snr)
{
  if (connected != lastHeltecConnected)
  {
    heltecConnected = connected;
    lastHeltecConnected = connected;
    renderStatusBar();
  }
}

void Screen::drawCursor()
{
  tft.fillRect(cursorX, cursorY, CURSOR_WIDTH, CURSOR_HEIGHT, GREEN);
  cursorVisible = true;
}

void Screen::eraseCursor()
{
  if (!cursorVisible)
    return;
  tft.fillRect(cursorX, cursorY, CURSOR_WIDTH, CURSOR_HEIGHT, BLACK);
  cursorVisible = false;
}

void Screen::resetCursorBlink()
{
  if (cursorVisible)
  {
    eraseCursor();
    drawCursor();
  }
  lastCursorToggle = millis();
}

void Screen::updateCursor()
{
  unsigned long now = millis();
  if (now - lastCursorToggle >= CURSOR_BLINK_MS)
  {
    lastCursorToggle = now;
    cursorVisible ? eraseCursor() : drawCursor();
  }
}

void Screen::clear()
{
  tft.fillScreen(BLACK);
  renderStatusBar();
  cursorX = LEFT_MARGIN;
  cursorY = TOP_MARGIN;
  tft.setCursor(cursorX, cursorY);
  resetCursorBlink();
}

void Screen::newLine()
{
  eraseCursor();
  cursorX = LEFT_MARGIN;
  cursorY += LINE_HEIGHT;

  if (cursorY > SCREEN_HEIGHT - 20)
  {
    tft.fillScreen(BLACK);
    renderStatusBar();
    cursorX = LEFT_MARGIN;
    cursorY = TOP_MARGIN;
  }

  tft.setCursor(cursorX, cursorY);
  resetCursorBlink();
}

void Screen::printDefaultColorLine(const String &text)
{
  printLine(text, GREEN);
}

void Screen::printText(const String &text, uint16_t color)
{
  eraseCursor();
  tft.setTextColor(color);
  tft.setCursor(cursorX, cursorY);
  tft.print(text);

  cursorX += text.length() * CHAR_STEP;
  tft.setCursor(cursorX, cursorY);
  resetCursorBlink();
}

void Screen::printLine(const String &text, uint16_t color)
{
  eraseCursor();
  tft.setTextColor(color);
  tft.setCursor(LEFT_MARGIN, cursorY);
  tft.print(text);
  newLine();
}

void Screen::printPrompt()
{
  eraseCursor();
  tft.setTextColor(GREEN);
  tft.setCursor(cursorX, cursorY);
  tft.print("> ");
  cursorX += 10;
  tft.setCursor(cursorX, cursorY);
  resetCursorBlink();
}

void Screen::printChar(char c, uint16_t color)
{
  eraseCursor();
  tft.setTextColor(color);
  tft.setCursor(cursorX, cursorY);
  tft.print(c);

  cursorX += CHAR_STEP;

  if (cursorX > SCREEN_WIDTH - 20)
  {
    newLine();
  }
  else
  {
    tft.setCursor(cursorX, cursorY);
    resetCursorBlink();
  }
}

void Screen::eraseCharCell()
{
  tft.fillRect(cursorX, cursorY, CHAR_STEP, LINE_HEIGHT + 2, BLACK);
  tft.fillRect(cursorX + CHAR_STEP, cursorY, CHAR_STEP, LINE_HEIGHT + 2, BLACK);
  tft.setCursor(cursorX, cursorY);
}

void Screen::showBootScreenStep(const String &line)
{
  static int bootLineY = TOP_MARGIN + 50;

  if (bootLineY == TOP_MARGIN + 50)
  {
    clear();

    printLine("Silver Fox", WHITE);
    printLine("", GREEN);

    printLine("             .d888", YELLOW);
    printLine("            d88P\"", YELLOW);
    printLine("            888", YELLOW);
    printLine("            888888 .d88b.  888  888", YELLOW);
    printLine("            888   d88\"\"88b `Y8bd8P'", YELLOW);
    printLine("            888   888  888   X88K", YELLOW);
    printLine("            888   Y88..88P  .d8"
              "8b.",
              YELLOW);
    printLine("            888    \"Y88P\"  888  888", YELLOW);
    printLine("", GREEN);
  }

  tft.setTextColor(GREEN);
  tft.setCursor(LEFT_MARGIN, bootLineY);
  printLine(line, GREEN);

  bootLineY += LINE_HEIGHT;
  if (bootLineY > SCREEN_HEIGHT - 20)
  {
    bootLineY = TOP_MARGIN + 50;
  }
}

int Screen::getCursorX() const { return cursorX; }
int Screen::getCursorY() const { return cursorY; }
void Screen::setCursorX(int x) { cursorX = x; }
void Screen::setCursorY(int y) { cursorY = y; }