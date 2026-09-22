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

// Shared by both protocols. Fleets are flashed all at once, so this exists to
// make a mismatched flash visible, not to let two formats coexist
#define PROTOCOL_VERSION 1

// Device IDs are 0..NUM_DEVICE_IDS-1, and every ID-indexed table is this wide.
// It must stay below 256: IDs are uint8_t, and loops over the observation table
// use uint8_t counters
#define NUM_DEVICE_IDS 240

// Never a valid device ID, so it doubles as "unassigned"
#define UNASSIGNED_DEVICE_ID 255

// Ports are a WiFi concept, so they live here rather than on rvl::System
#define RVLA_PORT 4978
#define RVLI_PORT 4979

/*
RVLA: what a node displays. Channel scoped, sent by a controller

Signature: 4 bytes = "RVLA"
Version: 1 byte = PROTOCOL_VERSION
Destination: 1 byte = 0-239: individual device, 240-254: multicast, 255:
  broadcast
Source: 1 byte = the device ID of the sender
Packet type: 1 byte = 1: System, 3: Clock Sync, 4: Wave Animation
Channel: 1 byte = the channel this packet belongs to
Reserved: 1 byte
*/
#define PACKET_TYPE_SYSTEM 1
#define PACKET_TYPE_CLOCK_SYNC 3
#define PACKET_TYPE_WAVE_ANIMATION 4

// Broadcasts per clock sync set. Receivers map each broadcast to a slot with
// id % NUM_OBSERVATIONS_IN_SET, so senders and receivers must agree on it
#define NUM_OBSERVATIONS_IN_SET 3

/*
RVLI: what a node needs in order to participate at all, such as its identity.
Channel independent, and mostly originated by the transport coordinator

Signature: 4 bytes = "RVLI"
Version: 1 byte = PROTOCOL_VERSION
Source: 1 byte = sender's device ID, or 255 when it has none yet
Packet type: 1 byte = 1: ID Assignment
Reserved: 1 byte

ID Assignment payload:
Type: 1 byte = 1: request, 2: reply
Device ID: 1 byte = the assigned ID, reply only
*/
#define RVLI_PACKET_TYPE_ID_ASSIGNMENT 1

#define ID_REQUEST_TYPE 1
#define ID_REPLY_TYPE 2

extern uint8_t rvlaSignature[4];
extern uint8_t rvliSignature[4];

}  // namespace rvl

#endif  // RVL_CONFIG_H_
