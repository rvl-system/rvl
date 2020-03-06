/*
Copyright (c) Bryan Hughes <bryan@nebri.us>

This file is part of RVL Arduino.

RVL Arduino is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RVL Arduino is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RVL Arduino.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <limits.h>
#include <stdint.h>
#include "./rvl.h"
#include "./rvl/platform.h"
#include "./rvl/config.h"
#include "./rvl/protocols/network_state.h"
#include "./rvl/protocols/protocol.h"
#include "./rvl/protocols/system/system.h"

namespace ProtocolSystem {

#define SYNC_ITERATION_MODULO 1500
bool hasSyncedThisLoop = false;

void init() {
}

void loop() {
  if (Platform::platform->getDeviceMode() != RVLDeviceMode::Controller) {
    return;
  }
  if (millis() % CLIENT_SYNC_INTERVAL < SYNC_ITERATION_MODULO) {
    hasSyncedThisLoop = false;
    return;
  }
  if (hasSyncedThisLoop) {
    return;
  }
  hasSyncedThisLoop = true;
  sync();
}

/*
Power: 1 byte = the power state of the system. 0 = LEDs off, 1 = LEDs on, > 1 reserved
Brightness: 1 byte = the brightness of the system
Reserved: 2 bytes
*/

void sync() {
  if (Platform::platform->getDeviceMode() != RVLDeviceMode::Controller || !Platform::transport->isConnected()) {
    return;
  }
  rvl::debug("Syncing system parameters");
  Platform::transport->beginWrite(Protocol::getMulticastAddress());
  Protocol::sendMulticastHeader(PACKET_TYPE_SYSTEM);
  Platform::transport->write8(Platform::platform->getPowerState());
  Platform::transport->write8(Platform::platform->getBrightness());
  Platform::transport->write16(0);
  Platform::transport->endWrite();
}

void parsePacket(uint8_t source) {
  if (!NetworkState::isControllerNode(source)) {
    return;
  }
  rvl::debug("Parsing System packet");

  uint8_t power = Platform::transport->read8();  // power
  uint8_t brightness = Platform::transport->read8();  // brightness
  Platform::transport->read16();  // reserved

  Platform::platform->setPowerState(power);
  Platform::platform->setBrightness(brightness);
  return;
}

}  // namespace ProtocolSystem
