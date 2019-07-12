/*
Copyright (c) Bryan Hughes <bryan@nebri.us>

This file is part of Raver Lights Messaging.

Raver Lights Messaging is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Raver Lights Messaging is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Raver Lights Messaging.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <limits.h>
#include <stdint.h>
#include "./rvl.h"
#include "./rvl/protocols/giggle_pixel/giggle_pixel.h"
#include "./rvl/protocols/giggle_pixel/palette.h"
#include "./rvl/protocols/giggle_pixel/wave.h"
#include "./rvl/platform.h"

namespace GigglePixel {

#define PROTOCOL_VERSION 2
#define CHANNEL_OFFSET 240

void init() {
  Wave::init();
  Palette::init();
}

void loop() {
  Wave::loop();
}

void sync() {
  // TODO(nebrius): Probably need to determine device Mode as well as Wave vs Palette before calling this
  Wave::sync();
}

/*
Signature: 4 bytes = "GLPX"
Version: 1 byte = 2
Length: 2 bytes = the length of the payload
Type: 1 byte = 1:palette, 2:wave
Priority: 1 byte = not currently used or implemented
Destination: 1 byte = 0-239: individual device, 240-254: multicast, 255: broadcast
Source: 1 byte = the address of the device that sent the message
Reserved: 1 byte = reserved for future use
*/

void parsePacket() {
  Platform::logging->debug("Parsing GigglePixel packet");
  uint8_t version = Platform::transport->read8();
  if (version != PROTOCOL_VERSION) {
    Platform::logging->error("Received unsupported GigglePixel protocol version packet %d", version);
    return;
  }
  // TODO(nebrius): We don't use length cause the length is fixed for wave packets, but probably not for other types.
  // This approach also isn't very defensive coding and should be passed to Wave::parsePacket() for error handling.
  Platform::transport->read16();  // length.
  uint8_t packetType = Platform::transport->read8();
  Platform::transport->read8();  // priority
  uint8_t destination = Platform::transport->read8();  // destination
  uint8_t source = Platform::transport->read8();  // source
  Platform::transport->read8();  // reserved

  // Ignore our own packets
  if (source == Platform::platform->getDeviceId()) {
    return;
  }

  // Ignore multicast packets meant for a different multicast group
  if (
    destination >= CHANNEL_OFFSET && destination < 255 &&
    Platform::platform->getChannel() != destination - CHANNEL_OFFSET
  ) {
    return;
  }

  // Ignore unicast packets meant for a different destination
  if (destination < CHANNEL_OFFSET && destination != Platform::platform->getDeviceId()) {
    return;
  }

  switch (packetType) {
    case RVLPacketType::Wave:
      Wave::parsePacket();
      break;
    default:
      Platform::logging->error("Received unsupported GigglePixel packet type %d", packetType);
  }
}

void sendHeader(uint8_t address, uint8_t packetType, uint8_t priority, uint16_t length) {
  Platform::transport->write(const_cast<uint8_t*>(signature), sizeof(uint8_t) * 4);
  Platform::transport->write8(PROTOCOL_VERSION);
  Platform::transport->write16(length);
  Platform::transport->write8(packetType);
  Platform::transport->write8(priority);
  Platform::transport->write8(address);
  Platform::transport->write8(Platform::platform->getDeviceId());
  Platform::transport->write8(0);
}

void broadcastHeader(uint8_t packetType, uint8_t priority, uint16_t length) {
  sendHeader(255, packetType, priority, length);
}

void multicastHeader(uint8_t packetType, uint8_t priority, uint16_t length) {
  sendHeader(CHANNEL_OFFSET + Platform::platform->getChannel(), packetType, priority, length);
}

void unicastHeader(uint8_t address, uint8_t packetType, uint8_t priority, uint16_t length) {
  sendHeader(address, packetType, priority, length);
}

}  // namespace GigglePixel
