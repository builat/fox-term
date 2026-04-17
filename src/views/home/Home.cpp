#include "views/home/Home.h"
#include "views/app/App.h"
#include "Config.h"

void HomeView::onEnter(App &app)
{
    selectedIndex = 0;
}

void HomeView::render(App &app)
{
    Screen &screen = app.getScreen();
    DeviceState &state = app.getState();

    screen.clear();
    screen.printLine("== HOME ==", WHITE);
    screen.printLine("", WHITE);
    screen.printLine("Device name:    " + state.deviceName, GREEN);
    screen.printLine("Group name:     " + state.groupName, GREEN);
    screen.printLine("Frequency:      " + state.frequency, GREEN);
    screen.printLine("Encryption key: " + state.encryptionKey, GREEN);
    screen.printLine("Last RSSI:      " + state.lastRssi, GREEN);
    screen.printLine("Last SNR:       " + state.lastSnr, GREEN);
    screen.printLine("GPS MODULE:     To implement", GREEN);
    screen.printLine("Transport: " + state.lastTransportStatus, GREEN);
    screen.printLine("", GREEN);

    screen.printLine(String(selectedIndex == 0 ? "> " : "  ") + "Messages", YELLOW);
    screen.printLine(String(selectedIndex == 1 ? "> " : "  ") + "Settings", YELLOW);
    screen.printLine(String(selectedIndex == 2 ? "> " : "  ") + "Compose Message", YELLOW);
}

void HomeView::onKey(App &app, const KeyEvent &key)
{

    if (key.type == KEY_UP && selectedIndex > 0)
    {
        selectedIndex--;
        render(app);
    }
    else if (key.type == KEY_DOWN && selectedIndex < 2)
    {
        selectedIndex++;
        render(app);
    }
    else if (key.type == KEY_ENTER || key.type == KEY_SELECT || key.type == KEY_RIGHT)
    {
        switchToSelected(app);
    }
}

void HomeView::switchToSelected(App &app)
{
    switch (selectedIndex)
    {
    case 0:
        app.switchTo(VIEW_MESSAGES);
        break;
    case 1:
        app.switchTo(VIEW_SETTINGS_LIST);
        break;
    case 2:
        app.switchTo(VIEW_COMPOSE_MESSAGE);
        break;

    default:
        break;
    }
}