#pragma once
#include "views/View.h"

class SettingsEditView : public View
{
public:
    void onEnter(App &app) override;
    void onKey(App &app, const KeyEvent &key) override;
    void render(App &app) override;

private:
    String editBuffer;

    String getSettingLabel(int index);
    String getSettingValue(App &app, int index);
    void setSettingValue(App &app, int index, const String &value);
    int getMaxLength(int index);
};