#pragma once
#include "views/View.h"

class SettingsListView : public View
{
public:
    void onEnter(App &app) override;
    void onKey(App &app, const KeyEvent &key) override;
    void render(App &app) override;

private:
    int selectedIndex = 0;
};