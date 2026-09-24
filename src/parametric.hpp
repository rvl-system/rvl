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

#ifndef PARAMETRIC_H_
#define PARAMETRIC_H_

#include <stdint.h>

#define NUM_LAYERS 4

struct RVLColorComponent {
  uint8_t a = 0;
  uint8_t b = 0;
  int8_t w_t = 0;
  int8_t w_x = 0;
  int8_t phi = 0;
};

struct RVLLayer {
  RVLColorComponent h;
  RVLColorComponent s;
  RVLColorComponent v;
  RVLColorComponent a;
};

struct RVLParametricSettings {
  uint8_t timePeriod = 255;
  uint8_t distancePeriod = 32;
  RVLLayer layers[NUM_LAYERS];
};

#endif // PARAMETRIC_H_
