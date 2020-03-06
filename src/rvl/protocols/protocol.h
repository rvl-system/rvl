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

#ifndef RVL_PROTOCOLS_PROTOCOL_H_
#define RVL_PROTOCOLS_PROTOCOL_H_

#include <stdint.h>
#include "./rvl/platform.h"
#include "./rvl/config.h"

namespace rvl {

namespace Protocol {

// Note: we use the old style of enums here because we regularly switch between uint8_t values and these enum values
namespace RVLPacketType {
  enum RVLPacketType {
    Palette = 1,
    Wave = 2
  };
}

void init();
void loop();

void parsePacket();

uint8_t getMulticastAddress();

void sendHeader(uint8_t packetType, uint8_t destination);
void sendBroadcastHeader(uint8_t packetType);
void sendMulticastHeader(uint8_t packetType);

}  // namespace Protocol

}  // namespace rvl

#endif  // RVL_PROTOCOLS_PROTOCOL_H_
