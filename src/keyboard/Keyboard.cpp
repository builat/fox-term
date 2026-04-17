#include "Keyboard.h"
#include "Config.h"
#include <Wire.h>

void Keyboard::begin()
{
  Wire.begin();
}

KeyEvent Keyboard::read()
{
  Wire.requestFrom(KB_ADDR, 1);

  if (!Wire.available())
  {
    return {KEY_NONE, 0};
  }

  int8_t c = (int8_t)Wire.read();
  // if (c != 0)
  // {
  //   Serial.print("Received key: ");
  //   Serial.println(c, HEX);
  // }

  switch (c)
  {
  case 0:
    return {KEY_NONE, 0};
  case '\r':
  case '\n':
    return {KEY_ENTER, 0};
  case 0x08:
    return {KEY_BACKSPACE, 0};
  case (int8_t)0xB5:
    return {KEY_UP, 0};
  case (int8_t)0xB6:
    return {KEY_DOWN, 0};
  case (int8_t)0xB4:
    return {KEY_LEFT, 0};
  case (int8_t)0xB7:
    return {KEY_RIGHT, 0};
  case 0x09:
    return {KEY_SELECT, 0};
  case 0x1B:
    return {KEY_ESQ, 0};
  default:
    if (c >= 32 && c <= 126)
      return {KEY_CHAR, (char)c};
    return {KEY_NONE, 0};
  }
}