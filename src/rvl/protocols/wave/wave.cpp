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

#include <stdint.h>
#include <limits.h>
#include "./rvl.h"
#include "./wave.h"
#include "./rvl/platform.h"
#include "./rvl/config.h"
#include "./rvl/protocols/network_state.h"
#include "./rvl/protocols/protocol.h"
#include "./rvl/protocols/wave/wave.h"

namespace rvl {

namespace ProtocolWave {

/*
Time Period: 1 byte = The time period for each wave
Distance Period: 1 byte = The distance period for each wave
Wave1: 20 bytes = See Wave Parameters below
Wave2: 20 bytes = See Wave Parameters below
Wave3: 20 bytes = See Wave Parameters below
Wave4: 20 bytes = See Wave Parameters below

Wave Parameters:
h: a b w_t w_x phi
s: a b w_t w_x phi
v: a b w_t w_x phi
a: a b w_t w_x phi
*/

#define SYNC_ITERATION_MODULO 1750
bool hasSyncedThisLoop = false;

void init() {
  on(EVENT_WAVE_SETTINGS_UPDATED, sync);
}

void loop() {
  if (getDeviceMode() != DeviceMode::Controller) {
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

void sync() {
  if (getDeviceMode() != DeviceMode::Controller || !Platform::transport->isConnected()) {
    return;
  }
  debug("Syncing preset");
  auto waveSettings = getWaveSettings();
  uint16_t length = sizeof(RVLWave) * NUM_WAVES;
  Platform::transport->beginWrite(Protocol::getMulticastAddress());
  Protocol::sendMulticastHeader(PACKET_TYPE_WAVE_ANIMATION);
  Platform::transport->write8(waveSettings->timePeriod);
  Platform::transport->write8(waveSettings->distancePeriod);
  Platform::transport->write(reinterpret_cast<uint8_t*>(&(waveSettings->waves)), length);
  Platform::transport->endWrite();
}

void parsePacket(uint8_t source) {
  if (!NetworkState::isControllerNode(source)) {
    return;
  }
  debug("Parsing Wave packet");
  RVLWaveSettings newWaveSettings;
  newWaveSettings.timePeriod = Platform::transport->read8();
  newWaveSettings.distancePeriod = Platform::transport->read8();
  Platform::transport->read(reinterpret_cast<uint8_t*>(&newWaveSettings.waves), sizeof(RVLWave) * NUM_WAVES);
  setWaveSettings(&newWaveSettings);
}

}  // namespace ProtocolWave

}  // namespace rvl
