#pragma once

#include <Arduino.h>
#include <DIYables_TFT_Shield.h>

#define KB_ADDR 0x5F

static const uint16_t GREEN = DIYables_TFT::colorRGB(24, 133, 40);
static const uint16_t BLACK = DIYables_TFT::colorRGB(0, 0, 0);
static const uint16_t WHITE = DIYables_TFT::colorRGB(255, 255, 255);
static const uint16_t RED = DIYables_TFT::colorRGB(200, 40, 40);
static const uint16_t YELLOW = DIYables_TFT::colorRGB(255, 220, 0);

static const int STATUS_BAR_HEIGHT = 12;

static const int LEFT_MARGIN = 5;
static const int TOP_MARGIN = STATUS_BAR_HEIGHT + 8;
static const int LINE_HEIGHT = 10;
static const int SCREEN_WIDTH = 480;
static const int SCREEN_HEIGHT = 320;
static const int CHAR_STEP = 5;

static const unsigned long CURSOR_BLINK_MS = 400;
static const int CURSOR_WIDTH = 5;
static const int CURSOR_HEIGHT = 8;
