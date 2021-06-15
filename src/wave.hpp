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

#ifndef WAVE_H_
#define WAVE_H_

#include <stdint.h>

#define NUM_WAVES 4

struct RVLWaveChannel {
  uint8_t a = 0;
  uint8_t b = 0;
  int8_t w_t = 0;
  int8_t w_x = 0;
  int8_t phi = 0;
};

struct RVLWave {
  RVLWaveChannel h;
  RVLWaveChannel s;
  RVLWaveChannel v;
  RVLWaveChannel a;
};

struct RVLWaveSettings {
  uint8_t timePeriod = 255;
  uint8_t distancePeriod = 32;
  RVLWave waves[NUM_WAVES];
};

#endif // WAVE_H_
