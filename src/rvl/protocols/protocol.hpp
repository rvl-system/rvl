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

#ifndef RVL_PROTOCOLS_PROTOCOL_H_
#define RVL_PROTOCOLS_PROTOCOL_H_

#include "./rvl/config.hpp"
#include "./rvl/platform.hpp"
#include <stdint.h>

namespace rvl {

namespace Protocol {

// Note: we use the old style of enums here because we regularly switch between
// uint8_t values and these enum values
namespace RVLPacketType {
enum RVLPacketType { Palette = 1, Wave = 2 };
} // namespace RVLPacketType

void init();
void loop();

void parsePacket();

void beginBroadcastWrite(uint8_t packetType);
void beginMulticastWrite(uint8_t packetType);
void beginUnicastWrite(uint8_t packetType, uint8_t address);

} // namespace Protocol

} // namespace rvl

#endif // RVL_PROTOCOLS_PROTOCOL_H_
