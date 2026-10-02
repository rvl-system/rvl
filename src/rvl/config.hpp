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

// One render step, in animation clock ms. Frame numbers are animationClock /
// FRAME_PERIOD, so this must divide 2^32 for them to wrap with the clock
#define FRAME_PERIOD 32

// The shortest fade any node runs, whether asked for or catching up late
#define MIN_FADE_FRAMES 16

// A controller board's own changes. Activation and the parser both floor a
// smaller default silently
#define DEFAULT_FADE_FRAMES 16
static_assert(DEFAULT_FADE_FRAMES >= MIN_FADE_FRAMES);

// The lead: how far ahead of now a new scene starts, so its packet reaches
// every receiver before the start
#define SCENE_LEAD_FRAMES 5

// How long after a newly scheduled scene's first send its repeat goes out: more
// than a beacon interval, so the two copies ride different DTIM bursts, and
// before the scene starts
#define REPEAT_SEND_FRAMES 4
static_assert(REPEAT_SEND_FRAMES < SCENE_LEAD_FRAMES);

// A day. A start 2^26 frames back reads as the future, so senders re-key their
// current scene well before that
#define SCENE_MAX_AGE_FRAMES 2700000

// One per protocol. Fleets are flashed all at once, so these exist to make a
// mismatched flash visible, not to let two formats coexist
#define RVLA_VERSION 2
#define RVLI_VERSION 1

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
Version: 1 byte = RVLA_VERSION
Source: 1 byte = the device ID of the sender
Packet type: 1 byte = 1: Off, 4: Parametric Animation
Channel: 1 byte = the channel this packet belongs to
Reserved: 1 byte

Every packet type carries one scene, and its payload starts with the timing:
Start: 4 bytes = the frame number the scene starts at
Reserved: 1 byte = 0, so the fade can widen to 16 bits as a big-endian no-op
Fade: 1 byte = frames to dissolve from what is showing into this scene

Off has nothing more; Parametric Animation follows with its settings
*/
#define PACKET_TYPE_OFF 1
#define PACKET_TYPE_PARAMETRIC_ANIMATION 4

/*
RVLI: what a node needs in order to participate at all, such as its identity
and clock. Channel independent, and mostly originated by the transport
coordinator

Signature: 4 bytes = "RVLI"
Version: 1 byte = RVLI_VERSION
Source: 1 byte = sender's device ID, or 255 when it has none yet
Packet type: 1 byte = 1: ID Assignment, 2: Clock Sync
Reserved: 1 byte

ID Assignment payload:
Type: 1 byte = 1: request, 2: reply
Device ID: 1 byte = the assigned ID, reply only
*/
#define RVLI_PACKET_TYPE_ID_ASSIGNMENT 1
#define RVLI_PACKET_TYPE_CLOCK_SYNC 2

#define ID_REQUEST_TYPE 1
#define ID_REPLY_TYPE 2

// Broadcasts per clock sync set. Receivers map each broadcast to a slot with
// id % NUM_OBSERVATIONS_IN_SET, so senders and receivers must agree on it
#define NUM_OBSERVATIONS_IN_SET 3

extern uint8_t rvlaSignature[4];
extern uint8_t rvliSignature[4];

} // namespace rvl

#endif // RVL_CONFIG_H_
