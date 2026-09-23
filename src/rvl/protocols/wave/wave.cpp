/*
Copyright (c) Bryan Hughes <bryan@nebri.us>

This file is part of RVL.

RVL is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RVL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RVL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "./wave.hpp"
#include "./rvl.hpp"
#include "./rvl/platform.hpp"
#include "./rvl/protocols/network_state.hpp"
#include "./rvl/protocols/wave/wave.hpp"
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

void write() {
  auto& animation = Platform::system->animation();
  auto* waveSettings = getWaveSettings();
  uint16_t length = sizeof(RVLWave) * NUM_WAVES;
  animation.write8(waveSettings->timePeriod);
  animation.write8(waveSettings->distancePeriod);
  animation.write(reinterpret_cast<uint8_t*>(&(waveSettings->waves)), length);
}

void parsePacket(uint8_t source) {
  if (!NetworkState::isControllerNode(source)) {
    return;
  }
  debug("Parsing Wave packet");
  auto& animation = Platform::system->animation();
  RVLWaveSettings newWaveSettings;
  newWaveSettings.timePeriod = animation.read8();
  newWaveSettings.distancePeriod = animation.read8();
  animation.read(reinterpret_cast<uint8_t*>(&newWaveSettings.waves),
      sizeof(RVLWave) * NUM_WAVES);
  setWaveSettings(&newWaveSettings);
}

} // namespace ProtocolWave

} // namespace rvl
