#include "SettingsEdit.h"
#include "views/app/App.h"
#include "Config.h"
#include "models/Models.h"

String SettingsEditView::getSettingLabel(int index)
{
    switch (index)
    {
    case SettingId::SETTING_DEVICE_NAME:
        return "Device name";
    case SettingId::SETTING_GROUP_NAME:
        return "Group name";
    case SettingId::SETTING_FREQUENCY:
        return "Frequency";
    case SettingId::SETTING_ENCRYPTION_KEY:
        return "Encryption key";
    default:
        return "Unknown";
    }
}

String SettingsEditView::getSettingValue(App &app, int index)
{
    DeviceState &state = app.getState();

    switch (index)
    {
    case SettingId::SETTING_DEVICE_NAME:
        return state.deviceName;
    case SettingId::SETTING_GROUP_NAME:
        return state.groupName;
    case SettingId::SETTING_FREQUENCY:
        return state.frequency;
    case SettingId::SETTING_ENCRYPTION_KEY:
        return state.encryptionKey;
    default:
        return "";
    }
}

void SettingsEditView::setSettingValue(App &app, int index, const String &value)
{
    DeviceState &state = app.getState();

    switch (index)
    {
    case SettingId::SETTING_DEVICE_NAME:
        state.deviceName = value;
        break;
    case SettingId::SETTING_GROUP_NAME:
        state.groupName = value;
        break;
    case SettingId::SETTING_FREQUENCY:
        state.frequency = value;
        break;
    case SettingId::SETTING_ENCRYPTION_KEY:
        state.encryptionKey = value;
        break;
    }
}

int SettingsEditView::getMaxLength(int index)
{
    switch (index)
    {
    case SettingId::SETTING_DEVICE_NAME:
        return 16;
    case SettingId::SETTING_GROUP_NAME:
        return 16;
    case SettingId::SETTING_FREQUENCY:
        return 10;
    case SettingId::SETTING_ENCRYPTION_KEY:
        return 32;
    default:
        return 16;
    }
}

void SettingsEditView::onEnter(App &app)
{
    int idx = app.getState().selectedSettingIndex;
    editBuffer = getSettingValue(app, idx);
}

void SettingsEditView::render(App &app)
{
    Screen &screen = app.getScreen();
    int idx = app.getState().selectedSettingIndex;

    screen.clear();
    screen.printLine("== EDIT SETTING ==", WHITE);
    screen.printLine("", WHITE);
    screen.printLine(getSettingLabel(idx) + " Max len: " + String(getMaxLength(idx)), RED);
    screen.printLine("", WHITE);
    screen.printLine(editBuffer, WHITE);
    screen.printLine("", WHITE);
    screen.printLine("ENTER=save", YELLOW);
    screen.printLine("ESC=cancel", YELLOW);

    screen.setCursorX(4 + editBuffer.length() * 6);
    screen.setCursorY(60);
    screen.drawCursor();
    screen.resetCursorBlink();
}

void SettingsEditView::onKey(App &app, const KeyEvent &key)
{
    int idx = app.getState().selectedSettingIndex;

    if (key.type == KEY_ENTER)
    {
        setSettingValue(app, idx, editBuffer);
        app.getScreen().eraseCursor();
        app.switchTo(VIEW_SETTINGS_LIST);
        return;
    }

    if (key.type == KEY_LEFT || key.type == KEY_ESQ)
    {
        app.switchTo(VIEW_SETTINGS_LIST);
        app.getScreen().eraseCursor();
        return;
    }

    if (key.type == KEY_BACKSPACE)
    {
        if (editBuffer.length() > 0)
        {
            editBuffer.remove(editBuffer.length() - 1);
            render(app);
        }
        return;
    }

    if (key.type == KEY_CHAR)
    {
        if (editBuffer.length() < getMaxLength(idx))
        {
            char ch = key.ch;

            // light validation for frequency
            if (idx == SettingId::SETTING_FREQUENCY)
            {
                if (!((ch >= '0' && ch <= '9') || ch == '.'))
                {
                    return;
                }
            }

            editBuffer += ch;
            render(app);
        }
        return;
    }
}