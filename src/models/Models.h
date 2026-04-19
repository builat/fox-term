#pragma once
#include <Arduino.h>
#include "models/Messages.h"
#include "models/TransportMessage.h"
const int MAX_MESSAGES = 10;

struct Message
{
    String id;
    String from;
    String to;
    String title;
    String body;
    bool unread;
};

enum SettingId
{
    SETTING_DEVICE_NAME,
    SETTING_GROUP_NAME,
    SETTING_FREQUENCY,
    SETTING_ENCRYPTION_KEY,
    SETTING_COUNT
};

struct DeviceState
{
    String deviceName;
    String groupName;
    String frequency;
    String encryptionKey;

    bool heltecConnected;
    unsigned long uptimeMs;

    String lastRssi;
    String lastSnr;

    Message messages[MAX_MESSAGES];
    int messageCount;
    int selectedMessageIndex;
    int selectedSettingIndex;

    TransportMessage draftMessage;

    String lastTransportStatus;
    int nextMessageSeq;
};
