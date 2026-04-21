#include "App.h"
#include "views/home/Home.h"
#include "views/messages/Messages.h"
#include "views/message_details/MessageDetails.h"
#include "views/settings_list/SettingsList.h"
#include "views/settings_edit/SettingsEdit.h"
#include "views/compose_message/ComposeMessage.h"

// ---------------------------------------------------------------------------
// App
//
// Central coordinator that owns all views, the shared DeviceState, and
// references to the hardware subsystems.  Follows a simple Model-View
// pattern: DeviceState is the model; View subclasses render it and mutate it
// in response to key events.  App handles view transitions and bridges
// hardware events (radio status changes) into the state.
// ---------------------------------------------------------------------------

App::App(Screen &s, UartLink &u, SdStorage &sd)
    : screen(s), uart(u), storage(sd), dispatcher(u)
{
    // All views are allocated once at startup and reused for the device's
    // lifetime.  Using new avoids large objects on the stack and lets us pass
    // *this into onEnter/render without object slicing.
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
    // Compiled-in defaults — used when no SD card is present or config is
    // missing.  Overwritten below if the SD card has a valid config file.
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

    // Reset the outgoing draft to a clean template.
    state.draftMessage.from = state.deviceName;
    state.draftMessage.to = "";
    state.draftMessage.targetType = TARGET_DEVICE;
    state.draftMessage.useEncryption = false;
    state.draftMessage.includeGps = true;
    state.draftMessage.text = "";

    state.lastTransportStatus = "IDLE";
    state.nextMessageSeq = 1;

    // Override defaults with values saved on the SD card (if available).
    // Re-sync the draft "from" field in case deviceName changed.
    if (storage.readConfig(state))
        state.draftMessage.from = state.deviceName;

    switchTo(VIEW_HOME);
}

UartLink &App::getUart()
{
    return uart;
}

void App::handleKey(const KeyEvent &key)
{
    // Delegate the event entirely to whichever view is currently active.
    if (currentView)
    {
        currentView->onKey(*this, key);
    }
}

void App::setRadioStatus(bool connected, const String &rssi, const String &snr)
{
    // Keep the model in sync, then push the update to the screen's status bar.
    state.heltecConnected = connected;
    state.lastRssi = rssi;
    state.lastSnr = snr;
    screen.setRadioStatus(connected, rssi, snr);
}

// Switches the active view: calls onEnter() so the view can reset internal
// selection state, then render() to draw the initial scene.  The cursor is
// erased afterwards so it doesn't bleed into the freshly painted display.
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

SdStorage &App::getStorage()
{
    return storage;
}