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

#include <stdint.h>
#include "./rvl/platform.h"
#include "./rvl/config.h"
#include "./rvl/protocols/protocol.h"
#include "./rvl/protocols/system/system.h"
#include "./rvl/protocols/discover/discover.h"
#include "./rvl/protocols/clock_sync/clock_sync.h"
#include "./rvl/protocols/wave/wave.h"

namespace Protocol {

#define PROTOCOL_VERSION 1

/*
Signature: 4 bytes = "RVLX"
Version: 1 byte = PROTOCOL_VERSION
Destination: 1 byte = 0-239: individual device, 240-254: multicast, 255: broadcast
Source: 1 byte = the address of the device that sent the message
Packet type: 1 byte = 1: System, 2: Discover, 3: Clock Sync, 4: Wave Animation
Reserved: 2 bytes = Reserved for future use
*/

void init() {
  ProtocolSystem::init();
  ProtocolDiscover::init();
  ProtocolClockSync::init();
  ProtocolWave::init();
}

void loop() {
  ProtocolSystem::loop();
  ProtocolDiscover::loop();
  ProtocolClockSync::loop();
  ProtocolWave::loop();
}

uint8_t getMulticastAddress() {
  return CHANNEL_OFFSET + Platform::platform->getChannel();
}

void parsePacket() {
  uint8_t version = Platform::transport->read8();
  if (version != PROTOCOL_VERSION) {
    Platform::logging->error("Received unsupported Raver Lights protocol packet version %d, ignoring", version);
    return;
  }

  uint8_t destination = Platform::transport->read8();  // destination
  uint8_t source = Platform::transport->read8();  // source
  uint8_t packetType = Platform::transport->read8();
  Platform::transport->read16();  // reserved

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

  Platform::logging->debug("Processing Message sent from %d to %d", source, destination);
  switch (packetType) {
    case 1:
      ProtocolSystem::parsePacket();
      break;
    case 2:
      ProtocolDiscover::parsePacket();
      break;
    case 3:
      ProtocolClockSync::parsePacket();
      break;
    case 4:
      ProtocolWave::parsePacket();
      break;
    default:
      Platform::logging->debug("Received unknown subpacket type %d", packetType);
      break;
  }
}

void sendHeader(uint8_t packetType) {
  Platform::transport->write(signature, 4);
  Platform::transport->write8(PROTOCOL_VERSION);
  Platform::transport->write8(getMulticastAddress());
  Platform::transport->write8(Platform::platform->getDeviceId());
  Platform::transport->write8(packetType);
  Platform::transport->write16(0);
}

}  // namespace Protocol
