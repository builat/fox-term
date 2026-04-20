#pragma once
#include "screen/Screen.h"
#include "models/Models.h"
#include "views/View.h"
#include "uart/UartLink.h"
#include "views/compose_message/ComposeMessage.h"
#include "transport/MessageDispatcher.h"
#include "storage/SdStorage.h"

class HomeView;
class MessagesView;
class MessageDetailView;
class SettingsListView;
class SettingsEditView;

enum ViewId
{
    VIEW_HOME,
    VIEW_MESSAGES,
    VIEW_MESSAGE_DETAIL,
    VIEW_SETTINGS_LIST,
    VIEW_SETTING_EDIT,
    VIEW_COMPOSE_MESSAGE
};

class App
{
public:
    App(Screen &screen, UartLink &uart, SdStorage &storage);

    void begin();
    void handleKey(const KeyEvent &key);
    void setHeltecConnected(bool connected);
    void setRadioStatus(bool connected, const String &rssi, const String &snr);
    void switchTo(ViewId id);
    Screen &getScreen();
    UartLink &getUart();
    DeviceState &getState();
    MessageDispatcher &getDispatcher();
    SdStorage &getStorage();

private:
    Screen &screen;
    UartLink &uart;
    SdStorage &storage;
    MessageDispatcher dispatcher;
    DeviceState state;
    HomeView *homeView;
    MessagesView *messagesView;
    MessageDetailView *messageDetailView;
    SettingsListView *settingsListView;
    SettingsEditView *settingsEditView;
    ComposeMessageView *composeMessageView;
    View *currentView;
};
