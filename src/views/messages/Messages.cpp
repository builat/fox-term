#include "views/messages/Messages.h"
#include "views/app/App.h"
#include "Config.h"

void MessagesView::onEnter(App &app)
{
    selectedIndex = app.getState().selectedMessageIndex;
}

void MessagesView::render(App &app)
{
    Screen &screen = app.getScreen();
    DeviceState &state = app.getState();

    screen.clear();
    screen.printLine("== MESSAGES ==", WHITE);
    screen.printLine("", GREEN);

    for (int i = 0; i < state.messageCount; i++)
    {
        String prefix = (i == selectedIndex) ? "> " : "  ";
        String unread = state.messages[i].unread ? "*" : " ";
        screen.printLine(prefix + unread + " " + state.messages[i].title, GREEN);
    }

    screen.printLine("", GREEN);
    screen.printLine("ENTER/RIGHT=open", YELLOW);
    screen.printLine("ESC/LEFT=back", YELLOW);
}

void MessagesView::onKey(App &app, const KeyEvent &key)
{
    DeviceState &state = app.getState();

    if (key.type == KEY_UP && selectedIndex > 0)
    {
        selectedIndex--;
        render(app);
    }
    else if (key.type == KEY_DOWN && selectedIndex < state.messageCount - 1)
    {
        selectedIndex++;
        render(app);
    }
    else if (key.type == KEY_ENTER || key.type == KEY_SELECT || key.type == KEY_RIGHT)
    {
        state.selectedMessageIndex = selectedIndex;
        app.switchTo(VIEW_MESSAGE_DETAIL);
    }
    else if (key.type == KEY_LEFT || key.type == KEY_ESQ)
    {
        app.switchTo(VIEW_HOME);
    }
}