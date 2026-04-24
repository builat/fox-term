# fox-term

`fox-term` is an embedded terminal and messaging firmware for an Arduino Mega based user-interface device.

The firmware is designed as the UI half of a two-board field communication system:

- **Arduino Mega**: display rendering, keyboard input, local UI, message composition, message storage, UART link management.
- **Heltec ESP32 LoRa node**: radio transport, GPS acquisition, encryption, LoRa send/receive, network-side processing.

This repository currently contains the Arduino Mega / terminal-side firmware.

## Project Status

This project is under active development.

Current implemented areas:

- TFT screen abstraction.
- CardKB keyboard input abstraction.
- View-based UI architecture.
- Message composition flow.
- Chunked message protocol.
- UART transport to a Heltec radio node.
- Incoming message line processing and reassembly.
- Early SD-backed storage abstraction.
- Multiple PlatformIO board environments.

Planned or partially implemented areas:

- SD-backed inbox / outbox / sent persistence.
- Full bidirectional chunk protocol over radio.
- Message delivery state tracking.
- Encryption integration.
- GPS metadata integration.
- Cyrillic font and input support.

## System Architecture

The system intentionally separates UI responsibilities from radio/network responsibilities.

```text
+-------------------+          UART           +----------------------+
| Arduino Mega      | <---------------------> | Heltec ESP32 LoRa    |
|                   |                         |                      |
| TFT UI            |                         | LoRa radio           |
| Keyboard input    |                         | GPS                  |
| Message composer  |                         | Encryption           |
| SD storage        |                         | Network transport    |
+-------------------+                         +----------------------+
```

The Arduino Mega does not perform radio operations directly. It prepares messages, manages local state, and communicates with the Heltec node through a line-based UART protocol.

## Design Principles

### 1. Separation of Responsibilities

The Mega firmware is responsible for human interaction and local persistence only.

The Heltec firmware is responsible for transport-specific concerns:

- radio send/receive,
- GPS tagging,
- encryption,
- low-level network state.

This avoids coupling display code to radio behavior and keeps each board replaceable.

### 2. Explicit Protocol Boundaries

Communication between subsystems uses explicit text frames.

The current message transport protocol uses chunked frames:

```text
MSG|<message-id>|<chunk-type>|<chunk-meta>|<payload>
```

Example:

```text
MSG|1000-7|HDR||from=node-a;to=group-a;mode=group
MSG|1000-7|GPS||1
MSG|1000-7|ENC||0
MSG|1000-7|TXT|1/2|Hello from the fi
MSG|1000-7|TXT|2/2|eld terminal
MSG|1000-7|END||
```

The protocol is intentionally readable during development. Debuggability is preferred over binary efficiency at this stage.

**Protocol will be changed to more optimized after system will be stable and ready for prod tests**

### 3. Chunked Transport

Messages are not sent as one large UART line.

Large single-line payloads are unsafe because they can exceed UART buffers, get truncated, or become difficult to validate. The firmware therefore splits logical messages into smaller chunks and reassembles them by message ID.

Each chunk carries the message ID. This prevents mixing data from different messages.

### 4. Message Object First

The UI operates on message objects, not raw transport lines.

A message has:

- sender,
- recipient,
- target type,
- encryption flag,
- GPS flag,
- text,
- generated message ID.

Transport encoding is a separate concern handled by protocol classes.

### 5. Storage Is Optional

The firmware must work without an SD card.

When SD is unavailable, messages can still be sent directly to the Heltec node. When SD support is available, messages should be written to local queues first.

Expected storage layout:

```text
/messages/
  inbox/
  outbox/
  sent/
```

Each message should be stored as an individual file.

### 6. Views Own UI Behavior

The UI is organized as views. Views handle local input and rendering for one screen.

Examples:

- home/status view,
- message list view,
- message details view,
- compose view,
- settings list view,
- settings edit view.

Rendering primitives remain in the `screen` layer.

### 7. Hardware Access Is Wrapped

Direct hardware access should stay behind small abstractions.

Examples:

- `Keyboard` wraps CardKB input.
- `Screen` wraps TFT output.
- `UartLink` wraps Mega-to-Heltec serial communication.
- `SdStorage` wraps SD persistence.
- `MessageDispatcher` wraps outgoing message transport.
- `LineProcessor` wraps incoming UART line handling.

Application logic should not directly access low-level peripherals unless there is a clear reason.

## Repository Layout

Current source layout:

```text
src/
  keyboard/     CardKB input handling
  models/       Device, message, and transport data models
  protocol/     Message chunk encoding, decoding, and assembly
  screen/       TFT rendering abstraction
  storage/      SD storage abstraction
  transport/    Message dispatch and incoming line processing
  uart/         UART communication with Heltec
  utils/        Utility code
  views/        View-based UI implementation
  Config.h      Shared configuration constants
  main.cpp      Firmware entry point
```

Top-level PlatformIO project files:

```text
platformio.ini
include/
lib/
test/
```

## Protocol

### Chunk Frame

General format:

```text
MSG|<id>|<type>|<meta>|<payload>
```

Fields:

| Field | Description |
|---|---|
| `id` | Message identifier. Must be present on every chunk. |
| `type` | Chunk type. |
| `meta` | Chunk-specific metadata. |
| `payload` | Chunk data. |

### Chunk Types

| Type | Meaning |
|---|---|
| `HDR` | Message header. Contains `from`, `to`, and target mode. |
| `GPS` | GPS tagging flag. Payload is `1` or `0`. |
| `ENC` | Encryption flag. Payload is `1` or `0`. |
| `TXT` | Text fragment. Metadata contains `<index>/<total>`. |
| `END` | End of message marker. |

### Text Chunk Numbering

Text chunk numbering is expected to be consistent between encoder and assembler.

Recommended convention:

```text
TXT|1/3
TXT|2/3
TXT|3/3
```

That is one-based numbering for human-readable logs.

### Header Payload

Header payload format:

```text
from=<device>;to=<target>;mode=<device|group>
```

Example:

```text
from=node-a;to=group-a;mode=group
```

## Message Acceptance Rule

An incoming complete message is accepted only when:

```cpp
message.to == deviceName || message.to == groupName
```

If the message target does not match the local device name or group name, the message is ignored.

The Mega side is responsible for final accept/store decisions. (**WILL BE CHANGED IN FUTURE**)

## Build System

The project uses PlatformIO.

The repository supports multiple board environments, including:

- regular Arduino Mega / Mega ADK style targets,
- Arduino Mega WiFi / CH340 based targets,
- native test environment.

Use the PlatformIO environment selector in VS Code to choose the active target before building or uploading.

Typical actions:

```text
PlatformIO: Build
PlatformIO: Upload
PlatformIO: Monitor
```

If using the CLI:

```bash
pio run -e <environment>
pio run -e <environment> -t upload
pio device monitor -e <environment>
```

## Hardware Notes

### Arduino Mega UART

The Mega uses fixed hardware serial pins.

Recommended Heltec link:

```text
Mega TX1 pin 18 -> Heltec RX
Mega RX1 pin 19 <- Heltec TX
GND             -> GND
```

Mega TX is 5 V. Heltec RX is 3.3 V. Use a level shifter or resistor divider for the Mega TX to Heltec RX path.

### Mega WiFi Boards

Mega WiFi boards are often Mega 2560 + ESP8266 + CH340 boards. USB routing may depend on DIP switch settings. Use the board mode that connects USB to the ATmega2560 when uploading Mega firmware.

### TFT Shield

The TFT shield is used for rendering. Its SD slot may not be reliable on Mega-compatible shield variants because SD wiring can depend on SPI routing. External SD modules are preferred for reliable storage.

## Development Rules

1. Do not include `.cpp` files from other `.cpp` files.
2. Headers declare interfaces. Source files define implementations.
3. Keep peripheral access isolated in wrapper classes.
4. Do not pass raw UART lines into UI views.
5. Do not build application behavior into protocol parsers.
6. Keep protocol frames short and printable.
7. Do not silently truncate messages. Reject or chunk instead.
8. Make sender and receiver indexing conventions identical.
9. Keep storage optional. UI must still work without SD.
10. Prefer explicit state transitions over implicit side effects.

## Current Data Flow

### Outgoing Message

```text
ComposeView
  -> TransportMessage
  -> MessageDispatcher
  -> MessageChunkCodec
  -> UartLink
  -> Heltec
```

### Incoming Message

```text
Heltec
  -> UartLink
  -> LineProcessor
  -> MessageChunkCodec
  -> MessageAssembler
  -> accept/skip rule
  -> RAM inbox or SD storage
```

## Testing Strategy

The protocol layer should be tested independently from hardware.

Recommended test targets:

- chunk encoding,
- chunk decoding,
- message reassembly,
- malformed frame rejection,
- incomplete message rejection,
- target filtering,
- storage path generation.

The `native` PlatformIO environment can be used for host-side protocol tests where Arduino hardware is not required.

## Known Limitations

- Unicode / Cyrillic rendering is not complete.
- Long-term SD-backed queues are still under development.
- Delivery acknowledgement and retry logic are not finalized.
- Radio chunk retransmission is not implemented.
- Encryption is modeled as a flag but not fully implemented.
- GPS is represented as a message option; live GPS attachment is expected to happen on the Heltec side.

## Roadmap

Short-term:

- [ ] stabilize chunk protocol in both directions,
- [ ] finish incoming message storage path,
- [ ] add outbox and sent movement logic,
- [ ] add better send status reporting,
- [ ] add storage-backed message list.

Medium-term:

- [ ] implement Heltec-side chunked radio transport,
- [ ] add GPS metadata attachment,
- [ ] add encryption handling,
- [ ] add resend and retry policy,
- [ ] add message delivery acknowledgements.

Long-term:

- [ ] Russian UI rendering,
- [ ] Russian input mode,
- [ ] contact list,
- [ ] group management,
- [ ] robust offline queueing,
- [ ] multi-node routing.
