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

#ifndef RVL_CONFIG_H_
#define RVL_CONFIG_H_

#include <stdint.h>

namespace rvl {

#define NUM_NODES 240

#define CLIENT_SYNC_INTERVAL 2000
#define CHANNEL_OFFSET 240

// Packet type: 1 byte = 1: System, 2: Discover, 3: Clock Sync, 4: Wave Animation
#define PACKET_TYPE_SYSTEM 1
#define PACKET_TYPE_DISCOVER 2
#define PACKET_TYPE_CLOCK_SYNC 3
#define PACKET_TYPE_WAVE_ANIMATION 4

extern uint8_t signature[4];

}  // namespace rvl

#endif  // RVL_CONFIG_H_
