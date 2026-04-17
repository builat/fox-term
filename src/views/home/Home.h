#pragma once
#include "views/View.h"

class HomeView : public View
{
public:
    void onEnter(App &app) override;
    void onKey(App &app, const KeyEvent &key) override;
    void render(App &app) override;

private:
    int selectedIndex = 0;
    void switchToSelected(App &app);
};