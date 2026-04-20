#include "storage/SdStorage.h"

const char * const SdStorage::INBOX  = "/inbox";
const char * const SdStorage::OUTBOX = "/outbox";
const char * const SdStorage::SENT   = "/sent";

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
// Config  (/config.cfg  key=value per line, '#' starts a comment)
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

        if      (key == "deviceName")    state.deviceName    = val;
        else if (key == "groupName")     state.groupName     = val;
        else if (key == "frequency")     state.frequency     = val;
        else if (key == "encryptionKey") state.encryptionKey = val;
    }

    f.close();
    return true;
}

bool SdStorage::writeConfig(const DeviceState &state)
{
    if (!sdAvailable)
        return false;

    // Arduino SD opens FILE_WRITE in append mode, so remove first.
    SD.remove("/config.cfg");
    File f = SD.open("/config.cfg", FILE_WRITE);
    if (!f)
        return false;

    f.println("deviceName="    + state.deviceName);
    f.println("groupName="     + state.groupName);
    f.println("frequency="     + state.frequency);
    f.println("encryptionKey=" + state.encryptionKey);
    f.close();
    return true;
}

// ---------------------------------------------------------------------------
// Messages  (each stored as  folder/id.msg  with key=value lines)
// ---------------------------------------------------------------------------

static String msgPath(const char *folder, const String &from, const String &id)
{
    return String(folder) + "/" + from + "_" + id + ".msg";
}

bool SdStorage::saveMessage(const char *folder, const TransportMessage &msg)
{
    if (!sdAvailable)
        return false;

    String path = msgPath(folder, msg.from, msg.id);
    SD.remove(path.c_str());
    File f = SD.open(path.c_str(), FILE_WRITE);
    if (!f)
        return false;

    f.println("id="         + msg.id);
    f.println("from="       + msg.from);
    f.println("to="         + msg.to);
    f.println("targetType=" + String(msg.targetType == TARGET_GROUP ? "group" : "device"));
    f.println("encrypt="    + String(msg.useEncryption ? "1" : "0"));
    f.println("gps="        + String(msg.includeGps    ? "1" : "0"));
    f.println("text="       + msg.text);
    f.close();
    return true;
}

bool SdStorage::saveOutbox(const TransportMessage &msg) { return saveMessage(OUTBOX, msg); }
bool SdStorage::saveInbox(const TransportMessage &msg)  { return saveMessage(INBOX,  msg); }
bool SdStorage::saveSent(const TransportMessage &msg)   { return saveMessage(SENT,   msg); }

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
                ids[count++] = name.substring(0, name.length() - 4);
        }
        entry.close();
    }

    dir.close();
    return count;
}

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

        if      (key == "id")   out.id   = val;
        else if (key == "from") out.from = val;
        else if (key == "to")   out.to   = val;
        else if (key == "text") out.body = val;
    }

    out.title  = "[ " + out.from + " ] [ " + out.id + " ]";
    out.unread = true;
    f.close();
    return out.id.length() > 0;
}
