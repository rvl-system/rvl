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

#define CLIENT_SYNC_INTERVAL 2000
#define CHANNEL_OFFSET 240

#define PROTOCOL_VERSION 1

// Packet type: 1 byte = 1: System, 3: Clock Sync, 4: Wave Animation
#define PACKET_TYPE_SYSTEM 1
#define PACKET_TYPE_CLOCK_SYNC 3
#define PACKET_TYPE_WAVE_ANIMATION 4

// Broadcasts per clock sync set. Receivers map each broadcast to a slot with
// id % NUM_OBSERVATIONS_IN_SET, so senders and receivers must agree on it
#define NUM_OBSERVATIONS_IN_SET 3

/*
RVLI: the protocol nodes speak to the transport coordinator, as distinct from
RVLX above, which nodes speak to each other. The split is not cosmetic — every
RVLX packet carries a real source, and RVLI exists precisely to establish the
identity that source byte holds, so it cannot presuppose one. Its own port keeps
the two from draining each other's traffic, and its own version lets the two
evolve without forcing lockstep flashing.

Like RVLX it is a container: the header identifies a packet type, and each type
defines its own payload. Device ID assignment is the first; node enumeration and
configuration routing are the expected next ones.

Signature: 4 bytes = "RVLI"
Version: 1 byte = RVLI_VERSION
Packet type: 1 byte = 1: ID Assignment
Source: 1 byte = sender's device ID, or 255 when it has none yet
Reserved: 1 byte

ID Assignment payload:
Type: 1 byte = 1: request, 2: reply
Device ID: 1 byte = the assigned ID, reply only
*/
#define RVLI_PORT 4979
#define RVLI_VERSION 1

#define RVLI_PACKET_TYPE_ID_ASSIGNMENT 1

#define ID_REQUEST_TYPE 1
#define ID_REPLY_TYPE 2

// Never a valid device ID (the space is 0..239), so it doubles as "unassigned"
#define UNASSIGNED_DEVICE_ID 255

extern uint8_t rvlxSignature[4];
extern uint8_t rvliSignature[4];

}  // namespace rvl

#endif  // RVL_CONFIG_H_
