#include "views/message_details/MessageDetails.h"
#include "views/app/App.h"
#include "Config.h"

void MessageDetailView::onEnter(App &app)
{
    DeviceState &state = app.getState();

    if (state.selectedMessageIndex >= 0 && state.selectedMessageIndex < state.messageCount)
    {
        state.messages[state.selectedMessageIndex].unread = false;
    }
}

void MessageDetailView::render(App &app)
{
    Screen &screen = app.getScreen();
    DeviceState &state = app.getState();
    Message &msg = state.messages[state.selectedMessageIndex];

    screen.clear();
    screen.printLine("== MESSAGE ==", WHITE);
    screen.printLine("", GREEN);

    screen.printLine("From: " + msg.title, YELLOW);
    screen.printLine("", GREEN);
    screen.printLine(msg.body, GREEN);
    screen.printLine("", GREEN);
    screen.printLine("ESC/LEFT=back", YELLOW);
}

void MessageDetailView::onKey(App &app, const KeyEvent &key)
{
    if (key.type == KEY_LEFT || key.type == KEY_ESQ || key.type == KEY_ENTER)
    {
        app.switchTo(VIEW_MESSAGES);
    }
}