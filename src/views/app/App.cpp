#include "App.h"
#include "views/home/Home.h"
#include "views/messages/Messages.h"
#include "views/message_details/MessageDetails.h"
#include "views/settings_list/SettingsList.h"
#include "views/settings_edit/SettingsEdit.h"
#include "views/compose_message/ComposeMessage.h"

App::App(Screen &s, UartLink &u)
    : screen(s), uart(u), dispatcher(u)
{
    homeView = new HomeView();
    messagesView = new MessagesView();
    messageDetailView = new MessageDetailView();
    settingsListView = new SettingsListView();
    settingsEditView = new SettingsEditView();
    composeMessageView = new ComposeMessageView();
    currentView = nullptr;
}

void App::begin()
{
    state.deviceName = "fox1";
    state.groupName = "default";
    state.frequency = "868.0";
    state.encryptionKey = "secret123";

    state.heltecConnected = false;
    state.uptimeMs = 0;
    state.lastRssi = "NA";
    state.lastSnr = "NA";

    state.messageCount = 0;
    state.selectedMessageIndex = 0;
    state.selectedSettingIndex = 0;

    state.draftMessage.from = state.deviceName;
    state.draftMessage.to = "";
    state.draftMessage.targetType = TARGET_DEVICE;
    state.draftMessage.useEncryption = false;
    state.draftMessage.includeGps = true;
    state.draftMessage.text = "";

    state.lastTransportStatus = "IDLE";
    state.nextMessageSeq = 1;

    switchTo(VIEW_HOME);
}

UartLink &App::getUart()
{
    return uart;
}

void App::handleKey(const KeyEvent &key)
{
    if (currentView)
    {
        currentView->onKey(*this, key);
    }
}

void App::setRadioStatus(bool connected, const String &rssi, const String &snr)
{
    state.heltecConnected = connected;
    state.lastRssi = rssi;
    state.lastSnr = snr;
    screen.setRadioStatus(connected, rssi, snr);
}

void App::switchTo(ViewId id)
{
    switch (id)
    {
    case VIEW_HOME:
        currentView = homeView;
        break;

    case VIEW_MESSAGES:
        currentView = messagesView;
        break;

    case VIEW_MESSAGE_DETAIL:
        currentView = messageDetailView;
        break;

    case VIEW_SETTINGS_LIST:
        currentView = settingsListView;
        break;

    case VIEW_SETTING_EDIT:
        currentView = settingsEditView;
        break;

    case VIEW_COMPOSE_MESSAGE:
        currentView = composeMessageView;
        break;
    }

    if (currentView)
    {
        currentView->onEnter(*this);
        currentView->render(*this);
    }
    screen.eraseCursor();
}

Screen &App::getScreen()
{
    return screen;
}

DeviceState &App::getState()
{
    return state;
}

MessageDispatcher &App::getDispatcher()
{
    return dispatcher;
}
void App::setHeltecConnected(bool connected)
{
    setRadioStatus(connected, state.lastRssi, state.lastSnr);
}