#pragma once
#include "views/View.h"

enum ComposeField
{
    COMPOSE_TO,
    COMPOSE_TARGET_TYPE,
    COMPOSE_ENCRYPTION,
    COMPOSE_GPS_TAG,
    COMPOSE_TEXT,
    COMPOSE_SEND,
    COMPOSE_CANCEL
};

class ComposeMessageView : public View
{
public:
    void onEnter(App &app) override;
    void onKey(App &app, const KeyEvent &key) override;
    void render(App &app) override;

private:
    int selectedField = COMPOSE_TO;

    bool isEditableField() const;
    String &activeBuffer(App &app);

    void moveUp();
    void moveDown();

    void toggleCurrentField(App &app);
    void handleSend(App &app);
    void handleCancel(App &app);
};