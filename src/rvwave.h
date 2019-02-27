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

#ifndef RVWAVE_H_
#define RVWAVE_H_

#include <stdint.h>
#include "./rvconfig.h"

struct RVWaveChannel {
  uint8_t a = 0;
  uint8_t b = 0;
  int8_t w_t = 0;
  int8_t w_x = 0;
  int8_t phi = 0;
};

struct RVWave {
  RVWaveChannel h;
  RVWaveChannel s;
  RVWaveChannel v;
  RVWaveChannel a;
};

struct RVWaveSettings {
  uint8_t timePeriod = 255;
  uint8_t distancePeriod = 32;
  RVWave waves[NUM_WAVES];
};

#endif  // RVWAVE_H_
