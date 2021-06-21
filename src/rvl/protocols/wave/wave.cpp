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

#include "./wave.hpp"
#include "./rvl.hpp"
#include "./rvl/config.hpp"
#include "./rvl/platform.hpp"
#include "./rvl/protocols/network_state.hpp"
#include "./rvl/protocols/protocol.hpp"
#include "./rvl/protocols/wave/wave.hpp"
#include <limits.h>
#include <stdint.h>

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

void sync() {
  if (getDeviceMode() != DeviceMode::Controller ||
      !Platform::system->isConnected())
  {
    return;
  }
  debug("Syncing preset");
  auto* waveSettings = getWaveSettings();
  uint16_t length = sizeof(RVLWave) * NUM_WAVES;
  Platform::system->beginWrite(Protocol::getMulticastAddress());
  Protocol::sendMulticastHeader(PACKET_TYPE_WAVE_ANIMATION);
  Platform::system->write8(waveSettings->timePeriod);
  Platform::system->write8(waveSettings->distancePeriod);
  Platform::system->write(
      reinterpret_cast<uint8_t*>(&(waveSettings->waves)), length);
  Platform::system->endWrite();
}

void parsePacket(uint8_t source) {
  if (!NetworkState::isControllerNode(source)) {
    return;
  }
  debug("Parsing Wave packet");
  RVLWaveSettings newWaveSettings;
  newWaveSettings.timePeriod = Platform::system->read8();
  newWaveSettings.distancePeriod = Platform::system->read8();
  Platform::system->read(reinterpret_cast<uint8_t*>(&newWaveSettings.waves),
      sizeof(RVLWave) * NUM_WAVES);
  setWaveSettings(&newWaveSettings);
}

} // namespace ProtocolWave

} // namespace rvl
