#include "SettingsList.h"
#include "views/app/App.h"
#include "Config.h"

static const char *settingNames[] = {
    "Device name",
    "Group name",
    "Frequency",
    "Encryption key"};

void SettingsListView::onEnter(App &app)
{
    selectedIndex = app.getState().selectedSettingIndex;
}

int getSettingCount()
{
    return sizeof(settingNames) / sizeof(settingNames[0]);
}

void SettingsListView::render(App &app)
{
    Screen &screen = app.getScreen();

    screen.clear();
    screen.printLine("== SETTINGS ==", WHITE);
    screen.printLine("", GREEN);

    for (int i = 0; i < getSettingCount(); i++)
    {
        String prefix = (i == selectedIndex) ? "> " : "  ";
        screen.printLine(prefix + String(settingNames[i]), YELLOW);
    }

    screen.printLine("", GREEN);
    screen.printLine("ENTER/RIGHT=edit", YELLOW);
    screen.printLine("ESC/LEFT=back", YELLOW);
}

void SettingsListView::onKey(App &app, const KeyEvent &key)
{
    if (key.type == KEY_UP && selectedIndex > 0)
    {
        selectedIndex--;
        render(app);
    }
    else if (key.type == KEY_DOWN && selectedIndex < getSettingCount() - 1)
    {
        selectedIndex++;
        render(app);
    }
    else if (key.type == KEY_ENTER || key.type == KEY_RIGHT || key.type == KEY_SELECT)
    {
        app.getState().selectedSettingIndex = selectedIndex;
        app.switchTo(VIEW_SETTING_EDIT);
    }
    else if (key.type == KEY_LEFT || key.type == KEY_ESQ)
    {
        app.switchTo(VIEW_HOME);
    }
}