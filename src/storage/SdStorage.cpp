#include "storage/SdStorage.h"

const char *const SdStorage::INBOX = "/inbox";
const char *const SdStorage::OUTBOX = "/outbox";
const char *const SdStorage::SENT = "/sent";

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool SdStorage::begin(int chipSelect)
{
    sdAvailable = SD.begin(chipSelect);
    return sdAvailable;
}

bool SdStorage::isAvailable() const
{
    return sdAvailable;
}

// Creates the three message folders if they are not already present.
// Fails fast and returns false if any mkdir fails (e.g. card is read-only).
bool SdStorage::ensureFileStructure()
{
    if (!sdAvailable)
        return false;

    const char *dirs[] = {INBOX, OUTBOX, SENT};
    for (const char *dir : dirs)
    {
        if (!SD.exists(dir) && !SD.mkdir(dir))
            return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Config  (/config.cfg)
//
// Plain-text key=value file, one entry per line.  Lines starting with '#'
// and blank lines are ignored.  Example:
//
//   # Silver Fox device config
//   deviceName=fox1
//   groupName=default
//   frequency=868.0
//   encryptionKey=secret123
// ---------------------------------------------------------------------------

bool SdStorage::readConfig(DeviceState &state)
{
    if (!sdAvailable)
        return false;

    File f = SD.open("/config.cfg", FILE_READ);
    if (!f)
        return false;

    while (f.available())
    {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0 || line[0] == '#')
            continue;

        int sep = line.indexOf('=');
        if (sep < 0)
            continue;

        String key = line.substring(0, sep);
        String val = line.substring(sep + 1);

        if (key == "deviceName")
            state.deviceName = val;
        else if (key == "groupName")
            state.groupName = val;
        else if (key == "frequency")
            state.frequency = val;
        else if (key == "encryptionKey")
            state.encryptionKey = val;
    }

    f.close();
    return true;
}

bool SdStorage::writeConfig(const DeviceState &state)
{
    if (!sdAvailable)
        return false;

    // Arduino's SD library opens FILE_WRITE in append mode, so we must
    // remove the old file before writing to get a clean overwrite.
    SD.remove("/config.cfg");
    File f = SD.open("/config.cfg", FILE_WRITE);
    if (!f)
        return false;

    f.println("deviceName=" + state.deviceName);
    f.println("groupName=" + state.groupName);
    f.println("frequency=" + state.frequency);
    f.println("encryptionKey=" + state.encryptionKey);
    f.close();
    return true;
}

// ---------------------------------------------------------------------------
// Messages
//
// Each message is stored as a separate file named  <from>_<id>.msg  inside
// the relevant folder (inbox / outbox / sent).  Including the sender in the
// filename makes files human-readable on a PC and avoids collisions when two
// senders happen to generate the same numeric ID.
//
// File format — key=value lines (same scheme as config.cfg):
//   id=<message-id>
//   from=<sender>
//   to=<recipient>
//   targetType=device|group
//   encrypt=0|1
//   gps=0|1
//   text=<message body>
// ---------------------------------------------------------------------------

// Builds the full path for a message file: e.g. "/inbox/alice_1234-1.msg"
static String msgPath(const char *folder, const String &from, const String &id)
{
    return String(folder) + "/" + from + "_" + id + ".msg";
}

bool SdStorage::saveMessage(const char *folder, const TransportMessage &msg)
{
    if (!sdAvailable)
        return false;

    String path = msgPath(folder, msg.from, msg.id);

    // Remove any existing file with the same path before writing so we always
    // get a fresh file (SD FILE_WRITE appends; it does not truncate).
    SD.remove(path.c_str());
    File f = SD.open(path.c_str(), FILE_WRITE);
    if (!f)
        return false;

    f.println("id=" + msg.id);
    f.println("from=" + msg.from);
    f.println("to=" + msg.to);
    f.println("targetType=" + String(msg.targetType == TARGET_GROUP ? "group" : "device"));
    f.println("encrypt=" + String(msg.useEncryption ? "1" : "0"));
    f.println("gps=" + String(msg.includeGps ? "1" : "0"));
    f.println("text=" + msg.text);
    f.close();
    return true;
}

// Convenience wrappers — each delegates to the shared saveMessage().
bool SdStorage::saveOutbox(const TransportMessage &msg) { return saveMessage(OUTBOX, msg); }
bool SdStorage::saveInbox(const TransportMessage &msg) { return saveMessage(INBOX, msg); }
bool SdStorage::saveSent(const TransportMessage &msg) { return saveMessage(SENT, msg); }

// Iterates the folder and collects the base-names of all .msg files
// (i.e. the "<from>_<id>" part without the extension).
int SdStorage::listMessages(const char *folder, String ids[], int maxIds)
{
    if (!sdAvailable || maxIds <= 0)
        return 0;

    File dir = SD.open(folder);
    if (!dir)
        return 0;

    int count = 0;
    while (count < maxIds)
    {
        File entry = dir.openNextFile();
        if (!entry)
            break;

        if (!entry.isDirectory())
        {
            String name = String(entry.name());
            if (name.endsWith(".msg"))
                ids[count++] = name.substring(0, name.length() - 4); // strip ".msg"
        }
        entry.close();
    }

    dir.close();
    return count;
}

// Opens <folder>/<from>_<id>.msg and parses each key=value line into `out`.
// Returns false if the file doesn't exist or the id field is missing.
bool SdStorage::readMessage(const char *folder, const String &from, const String &id, Message &out)
{
    if (!sdAvailable)
        return false;

    String path = msgPath(folder, from, id);
    File f = SD.open(path.c_str(), FILE_READ);
    if (!f)
        return false;

    out = Message{};
    while (f.available())
    {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue;

        int sep = line.indexOf('=');
        if (sep < 0)
            continue;

        String key = line.substring(0, sep);
        String val = line.substring(sep + 1);

        if (key == "id")
            out.id = val;
        else if (key == "from")
            out.from = val;
        else if (key == "to")
            out.to = val;
        else if (key == "text")
            out.body = val;
    }

    // Reconstruct the display title used by the message list view.
    out.title = "[ " + out.from + " ] [ " + out.id + " ]";
    out.unread = true;
    f.close();

    // A non-empty id is the minimum requirement for a usable message.
    return out.id.length() > 0;
}
