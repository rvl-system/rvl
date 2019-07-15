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
#include "./rvl/protocols/rvl_system/rvl_system.h"
#include "./rvl/platform.h"
#include "./rvl/config.h"
#include "./rvl/config.h"

namespace RVLSystem {

uint32_t nextSyncTime = INT_MAX;

#define PROTOCOL_VERSION 1

void init() {
  nextSyncTime = Platform::platform->getLocalTime() + CLIENT_SYNC_INTERVAL / 4;
}

void loop() {
  if (Platform::platform->getLocalTime() < nextSyncTime) {
    return;
  }
  nextSyncTime = Platform::platform->getLocalTime() + CLIENT_SYNC_INTERVAL;
  sync();
}

/*
Signature: 4 bytes = "RVLS"
Version: 1 byte = 1
Destination: 1 byte = 0-239: individual device, 240-254: multicast, 255: broadcast
Source: 1 byte = the address of the device that sent the message
Power: 1 byte = the power state of the system. 0 = LEDs off, 1 = LEDs on, > 1 reserved
Brightness: 1 byte = the brightness of the system
Reserved: 3 byte
*/

void sync() {
  if (Platform::platform->getDeviceMode() != RVLDeviceMode::Controller) {
    return;
  }
  Platform::logging->debug("Syncing system parameters");
  Platform::transport->beginWrite();
  Platform::transport->write(const_cast<uint8_t*>(signature), sizeof(uint8_t) * 4);
  Platform::transport->write8(PROTOCOL_VERSION);
  Platform::transport->write8(255); // Always broadcast
  Platform::transport->write8(Platform::platform->getDeviceId());
  Platform::transport->write8(Platform::platform->getPowerState());
  Platform::transport->write8(Platform::platform->getBrightness());
  Platform::transport->write8(0);
  Platform::transport->write16(0);
  Platform::transport->endWrite();
}

void parsePacket() {
  Platform::logging->debug("Parsing RVL System packet");
  uint8_t version = Platform::transport->read8();
  if (version != PROTOCOL_VERSION) {
    return;
  }
  uint8_t destination = Platform::transport->read8(); // destination
  uint8_t source = Platform::transport->read8(); // source
  uint8_t power = Platform::transport->read8();  // power
  uint8_t brightness = Platform::transport->read8();  // brightness
  Platform::transport->read8();  // reserved
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

  Platform::platform->setPowerState(power);
  Platform::platform->setBrightness(brightness);

  return;
}

}  // namespace RVLSystem
