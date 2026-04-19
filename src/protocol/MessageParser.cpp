#include "protocol/MessageParser.h"

String MessageParser::getField(const String &payload, const String &key)
{
    String pattern = key + "=";
    int start = payload.indexOf(pattern);
    if (start < 0)
        return "";

    start += pattern.length();
    int end = payload.indexOf('|', start);
    if (end < 0)
        end = payload.length();

    return payload.substring(start, end);
}

TransportMessage MessageParser::parse(const String &payload)
{
    TransportMessage msg;
    msg.id = getField(payload, "id");
    msg.from = getField(payload, "from");
    msg.to = getField(payload, "to");

    String target = getField(payload, "target");
    msg.targetType = (target == "group") ? TARGET_GROUP : TARGET_DEVICE;

    msg.useEncryption = (getField(payload, "enc") == "1");
    msg.includeGps = (getField(payload, "gps") == "1");

    msg.text = getField(payload, "text");
    msg.valid =
        msg.id.length() > 0 &&
        msg.from.length() > 0 &&
        msg.to.length() > 0 &&
        msg.text.length() > 0;

    return msg;
}