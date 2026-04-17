#pragma once
#include "keyboard/Keyboard.h"

class App;

class View
{
public:
    virtual ~View() {}

    virtual void onEnter(App &app) = 0;
    virtual void onKey(App &app, const KeyEvent &key) = 0;
    virtual void render(App &app) = 0;
};