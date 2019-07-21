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
#include <limits.h>
#include "./rvl.h"
#include "./rvl/wave.h"
#include "./rvl/protocols/protocol.h"
#include "./rvl/protocols/wave/wave.h"
#include "./rvl/platform.h"
#include "./rvl/config.h"

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

uint32_t nextSyncTime = INT_MAX;

void sync();

void init() {
  nextSyncTime = Platform::platform->getLocalTime();
}

void loop() {
  if (Platform::platform->getLocalTime() < nextSyncTime) {
    return;
  }
  nextSyncTime = Platform::platform->getLocalTime() + CLIENT_SYNC_INTERVAL;
  sync();
}

void sync() {
  if (Platform::platform->getDeviceMode() != RVLDeviceMode::Controller) {
    return;
  }
  Platform::logging->debug("Syncing preset");
  auto waveSettings = Platform::platform->getWaveSettings();
  uint16_t length = sizeof(RVLWave) * NUM_WAVES;
  Platform::transport->beginWrite();
  Protocol::sendHeader(4);
  Platform::transport->write8(waveSettings->timePeriod);
  Platform::transport->write8(waveSettings->distancePeriod);
  Platform::transport->write(reinterpret_cast<uint8_t*>(&(waveSettings->waves)), length);
  Platform::transport->endWrite();
}

void parsePacket() {
  Platform::logging->debug("Parsing Wave packet");
  RVLWaveSettings newWaveSettings;
  newWaveSettings.timePeriod = Platform::transport->read8();
  newWaveSettings.distancePeriod = Platform::transport->read8();
  Platform::transport->read(reinterpret_cast<uint8_t*>(&newWaveSettings.waves), sizeof(RVLWave) * NUM_WAVES);
  Platform::platform->setWaveSettings(&newWaveSettings);
}

}  // namespace ProtocolWave
