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
#include "./rvl/protocols/protocol.h"
#include "./rvl/protocols/system/system.h"
#include "./rvl/platform.h"
#include "./rvl/config.h"

namespace ProtocolSystem {

uint32_t nextSyncTime = INT_MAX;

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
Power: 1 byte = the power state of the system. 0 = LEDs off, 1 = LEDs on, > 1 reserved
Brightness: 1 byte = the brightness of the system
Reserved: 2 bytes
*/

void sync() {
  if (Platform::platform->getDeviceMode() != RVLDeviceMode::Controller) {
    return;
  }
  Platform::logging->debug("Syncing system parameters");
  Platform::transport->beginWrite();
  Protocol::sendHeader(1);
  Platform::transport->write8(Platform::platform->getPowerState());
  Platform::transport->write8(Platform::platform->getBrightness());
  Platform::transport->write16(0);
  Platform::transport->endWrite();
}

void parsePacket() {
  Platform::logging->debug("Parsing System packet");

  uint8_t power = Platform::transport->read8();  // power
  uint8_t brightness = Platform::transport->read8();  // brightness
  Platform::transport->read16();  // reserved

  Platform::platform->setPowerState(power);
  Platform::platform->setBrightness(brightness);
  return;
}

}  // namespace ProtocolSystem