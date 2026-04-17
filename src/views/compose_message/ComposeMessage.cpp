#include "views/compose_message/ComposeMessage.h"
#include "views/app/App.h"
#include "Config.h"
#include "utils/IdGenerator.h"

void ComposeMessageView::onEnter(App &app)
{
    selectedField = COMPOSE_TO;
    app.getState().draftMessage.from = app.getState().deviceName;
}

bool ComposeMessageView::isEditableField() const
{
    return selectedField == COMPOSE_TO || selectedField == COMPOSE_TEXT;
}

String &ComposeMessageView::activeBuffer(App &app)
{
    DeviceState &state = app.getState();

    if (selectedField == COMPOSE_TO)
        return state.draftMessage.to;

    return state.draftMessage.text;
}

void ComposeMessageView::moveUp()
{
    if (selectedField > COMPOSE_TO)
        selectedField--;
}

void ComposeMessageView::moveDown()
{
    if (selectedField < COMPOSE_CANCEL)
        selectedField++;
}

void ComposeMessageView::toggleCurrentField(App &app)
{
    DeviceState &state = app.getState();
    OutgoingMessage &msg = state.draftMessage;

    switch (selectedField)
    {
    case COMPOSE_TARGET_TYPE:
        msg.targetType = (msg.targetType == TARGET_DEVICE) ? TARGET_GROUP : TARGET_DEVICE;
        break;

    case COMPOSE_ENCRYPTION:
        msg.useEncryption = !msg.useEncryption;
        break;

    case COMPOSE_GPS_TAG:
        msg.includeGps = !msg.includeGps;
        break;

    default:
        break;
    }
}

void ComposeMessageView::handleSend(App &app)
{
    DeviceState &state = app.getState();
    OutgoingMessage &msg = state.draftMessage;

    if (msg.to.length() == 0 || msg.text.length() == 0)
    {
        state.lastTransportStatus = "Compose: missing to/text";
        return;
    }

    msg.id = IdGenerator::makeMessageId(state.deviceName, state.nextMessageSeq++);
    msg.from = state.deviceName;
    msg.status = MSG_QUEUED;

    // SD will be inserted here later.
    // For now send directly.
    app.getDispatcher().dispatch(msg);

    state.lastTransportStatus = "Queued/Sent: " + msg.id;

    // reset draft
    state.draftMessage = OutgoingMessage();
    state.draftMessage.from = state.deviceName;
    state.draftMessage.targetType = TARGET_DEVICE;
    state.draftMessage.useEncryption = false;
    state.draftMessage.includeGps = true;

    app.switchTo(VIEW_MESSAGES);
}

void ComposeMessageView::handleCancel(App &app)
{
    app.switchTo(VIEW_HOME);
}

void ComposeMessageView::render(App &app)
{
    Screen &screen = app.getScreen();
    DeviceState &state = app.getState();
    OutgoingMessage &msg = state.draftMessage;

    screen.clear();
    screen.printLine("== COMPOSE ==", WHITE);

    screen.printLine(String(selectedField == COMPOSE_TO ? "> " : "  ") +
                         "To: " + msg.to,
                     YELLOW);

    screen.printLine(String(selectedField == COMPOSE_TARGET_TYPE ? "> " : "  ") +
                         "Target: " + String(msg.targetType == TARGET_GROUP ? "group" : "device"),
                     YELLOW);

    screen.printLine(String(selectedField == COMPOSE_ENCRYPTION ? "> " : "  ") +
                         "Encrypt: " + String(msg.useEncryption ? "ON" : "OFF"),
                     YELLOW);

    screen.printLine(String(selectedField == COMPOSE_GPS_TAG ? "> " : "  ") +
                         "GPS tag: " + String(msg.includeGps ? "ON" : "OFF"),
                     YELLOW);

    screen.printLine(String(selectedField == COMPOSE_TEXT ? "> " : "  ") +
                         "Text: " + msg.text,
                     GREEN);

    screen.printLine("", GREEN);
    screen.printLine(String(selectedField == COMPOSE_SEND ? "> " : "  ") + "Send", GREEN);
    screen.printLine(String(selectedField == COMPOSE_CANCEL ? "> " : "  ") + "Cancel", RED);
}

void ComposeMessageView::onKey(App &app, const KeyEvent &key)
{
    if (key.type == KEY_UP)
    {
        moveUp();
        render(app);
        return;
    }

    if (key.type == KEY_DOWN || key.type == KEY_TAB)
    {
        moveDown();
        render(app);
        return;
    }

    if (key.type == KEY_LEFT || key.type == KEY_ESQ)
    {
        handleCancel(app);
        return;
    }

    if (key.type == KEY_BACKSPACE)
    {
        if (isEditableField())
        {
            String &buf = activeBuffer(app);
            if (buf.length() > 0)
            {
                buf.remove(buf.length() - 1);
                render(app);
            }
        }
        return;
    }

    if (key.type == KEY_CHAR)
    {
        if (isEditableField())
        {
            String &buf = activeBuffer(app);
            if (buf.length() < 120)
            {
                buf += key.ch;
                render(app);
            }
        }
        return;
    }

    if (key.type == KEY_ENTER || key.type == KEY_SELECT || key.type == KEY_RIGHT)
    {
        if (selectedField == COMPOSE_SEND)
        {
            handleSend(app);
            return;
        }

        if (selectedField == COMPOSE_CANCEL)
        {
            handleCancel(app);
            return;
        }

        if (selectedField == COMPOSE_TARGET_TYPE ||
            selectedField == COMPOSE_ENCRYPTION ||
            selectedField == COMPOSE_GPS_TAG)
        {
            toggleCurrentField(app);
            render(app);
            return;
        }

        moveDown();
        render(app);
    }
}