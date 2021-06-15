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

#include "./rvl/protocols/system/system.hpp"
#include "./rvl.hpp"
#include "./rvl/config.hpp"
#include "./rvl/platform.hpp"
#include "./rvl/protocols/network_state.hpp"
#include "./rvl/protocols/protocol.hpp"
#include "./rvl/state.hpp"
#include <limits.h>
#include <stdint.h>

namespace rvl {

namespace ProtocolSystem {

#define SYNC_ITERATION_MODULO 1500
bool hasSyncedThisLoop = false;

void init() {
  on(EVENT_POWER_STATE_UPDATED, sync);
  on(EVENT_BRIGHTNESS_UPDATED, sync);
}

void loop() {
  if (getDeviceMode() != DeviceMode::Controller) {
    return;
  }
  if (Platform::system->localClock() % CLIENT_SYNC_INTERVAL <
      SYNC_ITERATION_MODULO)
  {
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
Power: 1 byte = the power state of the system. 0 = LEDs off, 1 = LEDs on, > 1
reserved Brightness: 1 byte = the brightness of the system Reserved: 2 bytes
*/

void sync() {
  if (getDeviceMode() != DeviceMode::Controller ||
      !Platform::system->isConnected())
  {
    return;
  }
  debug("Syncing system parameters");
  Platform::system->beginWrite(Protocol::getMulticastAddress());
  Protocol::sendMulticastHeader(PACKET_TYPE_SYSTEM);
  Platform::system->write8(getPowerState());
  Platform::system->write8(getBrightness());
  Platform::system->write16(0);
  Platform::system->endWrite();
}

void parsePacket(uint8_t source) {
  if (!NetworkState::isControllerNode(source)) {
    return;
  }
  debug("Parsing System packet");

  uint8_t power = Platform::system->read8(); // power
  uint8_t brightness = Platform::system->read8(); // brightness
  Platform::system->read16(); // reserved

  setPowerState(power);

  if (getRemoteBrightnessState()) {
    setBrightness(brightness);
  }
}

} // namespace ProtocolSystem

} // namespace rvl
