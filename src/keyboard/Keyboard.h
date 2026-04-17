#pragma once
#include <Arduino.h>

enum KeyType
{
  KEY_NONE,
  KEY_CHAR,
  KEY_ENTER,
  KEY_BACKSPACE,
  KEY_UP,
  KEY_ESQ,
  KEY_TAB,
  KEY_DOWN,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_SELECT,
};

struct KeyEvent
{
  KeyType type;
  char ch;
};

class Keyboard
{
public:
  void begin();
  KeyEvent read();
};