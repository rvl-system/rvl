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
#include "./protocols/giggle_pixel/wave.h"
#include "./protocols/giggle_pixel/giggle_pixel.h"
#include "./RaverLightsMessaging.h"
#include "./platform.h"
#include "./config.h"
#include "./wave.h"

namespace Wave {

uint32_t nextSyncTime = Platform::platform->getLocalTime();

void sync();

void init() {
}

void loop() {
  if (Platform::platform->getDeviceMode() != RVDeviceMode::Controller) {
    return;
  }
  if (Platform::platform->getLocalTime() < nextSyncTime) {
    return;
  }
  nextSyncTime = Platform::platform->getLocalTime() + CLIENT_SYNC_INTERVAL;
  sync();
}

void sync() {
  Platform::debug("Syncing preset");
  auto waveSettings = Platform::platform->getWaveSettings();
  uint16_t length = sizeof(RVWave) * NUM_WAVES;
  Platform::transport->beginWrite();
  GigglePixel::broadcastHeader(
    GigglePixelPacketTypes::Wave,
    0,  // Priority
    2 + length);
  Platform::transport->write8(waveSettings->timePeriod);
  Platform::transport->write8(waveSettings->distancePeriod);
  Platform::transport->write(reinterpret_cast<uint8_t*>(&(waveSettings->waves)), length);
  Platform::transport->endWrite();
}

void parsePacket() {
  Platform::debug("Parsing Wave packet");
  RVWaveSettings newWaveSettings;
  newWaveSettings.timePeriod = Platform::transport->read8();
  newWaveSettings.distancePeriod = Platform::transport->read8();
  Platform::transport->read(reinterpret_cast<uint8_t*>(&newWaveSettings.waves), sizeof(RVWave) * NUM_WAVES);
  Platform::platform->setWaveSettings(&newWaveSettings);
}

}  // namespace Wave
