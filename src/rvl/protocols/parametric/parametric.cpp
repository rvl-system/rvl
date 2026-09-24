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

#include "./parametric.hpp"
#include "./rvl.hpp"
#include "./rvl/platform.hpp"
#include "./rvl/protocols/network_state.hpp"
#include "./rvl/protocols/parametric/parametric.hpp"
#include <stdint.h>

namespace rvl {

namespace ProtocolParametric {

/*
Time Period: 1 byte = The time period for each layer
Distance Period: 1 byte = The distance period for each layer
Layer1: 20 bytes = See Layer Layout below
Layer2: 20 bytes = See Layer Layout below
Layer3: 20 bytes = See Layer Layout below
Layer4: 20 bytes = See Layer Layout below

Layer Layout, one color component per line:
h: a b w_t w_x phi
s: a b w_t w_x phi
v: a b w_t w_x phi
a: a b w_t w_x phi
*/

void write() {
  auto& animation = Platform::system->animation();
  auto* settings = getParametricSettings();
  uint16_t length = sizeof(RVLLayer) * NUM_LAYERS;
  animation.write8(settings->timePeriod);
  animation.write8(settings->distancePeriod);
  animation.write(reinterpret_cast<uint8_t*>(&(settings->layers)), length);
}

void parsePacket(uint8_t source) {
  if (!NetworkState::isControllerNode(source)) {
    return;
  }
  debug("Parsing Parametric packet");
  auto& animation = Platform::system->animation();
  RVLParametricSettings newSettings;
  newSettings.timePeriod = animation.read8();
  newSettings.distancePeriod = animation.read8();
  animation.read(reinterpret_cast<uint8_t*>(&newSettings.layers),
      sizeof(RVLLayer) * NUM_LAYERS);
  // The renderer divides by both periods
  if (newSettings.timePeriod == 0 || newSettings.distancePeriod == 0) {
    error("Received a parametric packet with a zero period, ignoring");
    return;
  }
  setParametricSettings(&newSettings);
}

} // namespace ProtocolParametric

} // namespace rvl
