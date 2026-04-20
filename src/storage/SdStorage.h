#pragma once
#include <Arduino.h>
#include <SD.h>
#include "models/Models.h"
#include "models/TransportMessage.h"

class SdStorage
{
public:
    // Folder path constants for use with listMessages() and readMessage().
    static const char * const INBOX;
    static const char * const OUTBOX;
    static const char * const SENT;

    // Initialises the SD card on the given chip-select pin.
    bool begin(int chipSelect);

    // Returns true if the card was successfully initialised.
    bool isAvailable() const;

    // Creates /inbox, /outbox, /sent if they do not already exist.
    // Additional structure checks can be added here later.
    bool ensureFileStructure();

    // Reads /config.cfg into state (deviceName, groupName, frequency, encryptionKey).
    // Returns false if the card is unavailable or the file cannot be opened.
    bool readConfig(DeviceState &state);

    // Writes current state fields to /config.cfg, overwriting any existing file.
    bool writeConfig(const DeviceState &state);

    // Persist a message to the appropriate folder.
    bool saveOutbox(const TransportMessage &msg);
    bool saveInbox(const TransportMessage &msg);
    bool saveSent(const TransportMessage &msg);

    // Fills ids[] with the message IDs present in folder (up to maxIds).
    // Returns the number of IDs found.
    int listMessages(const char *folder, String ids[], int maxIds);

    // Reads a single message file from folder/id.msg into out.
    // Returns false if the file does not exist or cannot be parsed.
    bool readMessage(const char *folder, const String &from, const String &id, Message &out);

private:
    bool sdAvailable = false;

    // Shared implementation used by saveOutbox/saveInbox/saveSent.
    bool saveMessage(const char *folder, const TransportMessage &msg);
};
