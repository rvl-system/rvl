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

#include "./rvl/protocols/clock_sync/clock_sync.hpp"
#include "./rvl.hpp"
#include "./rvl/config.hpp"
#include "./rvl/platform.hpp"
#include "./rvl/protocols/network_state.hpp"
#include "./rvl/protocols/protocol.hpp"
#include <algorithm>
#include <iterator>
#include <stdint.h>

namespace rvl {

namespace ProtocolClockSync {

#define CLOCK_SYNC_TYPE_P2P 1

#define NUM_REQUESTS 4

#define SYNC_ITERATION_MODULO 500
#define SYNC_ITERATION_MODULO_MAX 1400

#define SYNC_TIMEOUT (SYNC_ITERATION_MODULO_MAX - SYNC_ITERATION_MODULO)

uint32_t syncTimeout = 0;

/*
Packet:
Algorithm Type: 1 byte = 1: Point to Point (other types maybe coming soon)
Reserved: 1 byte

Algorithm Type 1 Subpacket:
Observation Number: 1 byte = The current observation
Reserved: 1 byte
c1 (Controller Observation 1): 4 bytes = The first clock of the controller node
r1 (Receiver Observation 1): 4 bytes = The first clock of the receiver node
...
cN (Controller Observation NUM_REQUESTS): 4 bytes = The fourth clock of the controller node
rN (Receiver Observation NUM_REQUESTS): 4 bytes = The fourth clock of the receiver node
*/

/* Algorithm Type 1
1. The controller initializes a packet with the observation number set to 0 and
  all clock observations set to 0
2. The controller processes the received/initialized packet
  2.1 The controller increments the observation number
  2.2 The controller sets the appropriate observation slot in the packet based
    on the observation number
  2.3 The controller sends the packet to the receiver
3. The receiver process the received packet
  3.1 The receiver sets the appropriate observation slot in the packet based on
    the observation number
  3.2 If the observation number equals NUM_REQUESTS, then we skip to step 4
  3.3 Otherwise, send the packet to the server and skip to step 2
4. The receiver processes the observations
  4.1. Calculate the periods between c1 and c2, c2 and c3, c3 and c4, r1 and r2,
    r2 and r3, r3 and r4, and take the median / 2 as P
  4.2. Calculate offsets On=rn-(cn+P) and find the median O
  4.3. Update the animation clock by adding O to the current animation clock
*/

void init() {
}

// TODO: if two controllers are on the same network, then they both think they
// started the synchronization, which just isn't true. I should switch this so
// that we don't use `getDeviceMode()` to determine what to do in this function.
void sendResponse(
    uint8_t node, uint8_t observationNumber, uint32_t* observations) {
  if (getDeviceMode() == DeviceMode::Controller) {
    observationNumber++;
    observations[(observationNumber - 1) * 2] = getAnimationClock();
    if (observationNumber == NUM_REQUESTS) {
      NetworkState::refreshNodeClockSyncTime(node);
      syncTimeout = 0;
    }
  } else {
    if (!NetworkState::isControllerNode(node)) {
      return;
    }
    observations[(observationNumber - 1) * 2 + 1] = getAnimationClock();
    if (observationNumber == NUM_REQUESTS) {
      debug("Processing clock sync observation #%d", observationNumber, node);

      // Step 4.1
      int32_t periods[(NUM_REQUESTS - 1) * 2];
      for (int i = 0; i < NUM_REQUESTS * 2 - 3; i++) {
        periods[i] = static_cast<int32_t>(observations[i + 2]) -
            static_cast<int32_t>(observations[i]);
      }
      std::sort(std::begin(periods), std::end(periods));
      int16_t P = periods[NUM_REQUESTS - 1] / 2;

      // Step 4.2
      int32_t offsets[NUM_REQUESTS];
      for (uint8_t i = 0; i < NUM_REQUESTS; i++) {
        offsets[i] = static_cast<int32_t>(observations[i * 2 + 1]) -
            static_cast<int32_t>(observations[i * 2] + P);
      }
      int32_t offset = offsets[(NUM_REQUESTS - 1) / 2];

      // Step 4.3
      setAnimationClock(getAnimationClock() - offset);
      NetworkState::refreshLocalClockSynchronization();
      return;
    }
  }
  debug("Sending clock sync observation #%d to source %d", observationNumber,
      node);
  Protocol::beginUnicastWrite(PACKET_TYPE_CLOCK_SYNC, node);
  Platform::system->write8(CLOCK_SYNC_TYPE_P2P);
  Platform::system->write8(0); // reserved
  Platform::system->write8(observationNumber);
  Platform::system->write8(0); // reserved
  Platform::system->write(reinterpret_cast<uint8_t*>(observations),
      NUM_REQUESTS * 2 * sizeof(uint32_t));
  Platform::system->endWrite();
}

void loop() {
  if (getDeviceMode() != DeviceMode::Controller ||
      !Platform::system->isConnected())
  {
    return;
  }

  // Check if we're in our alloted time window
  if (Platform::system->localClock() % CLIENT_SYNC_INTERVAL <
          SYNC_ITERATION_MODULO ||
      Platform::system->localClock() % CLIENT_SYNC_INTERVAL >
          SYNC_ITERATION_MODULO_MAX)
  {
    return;
  }

  // Check if a synchronization is currently in progress
  if (syncTimeout > 0) {
    // Check if the synchronization has timed out
    if (Platform::system->localClock() > syncTimeout) {
      // If so, reset the timeout and get the next node
      info("Clock sync with node timed out");
      syncTimeout = 0;
    } else {
      // Otherwise skip
      return;
    }
  }

  // Get the next node to sync
  uint8_t nextNodeToSync = NetworkState::getNextClockNode();
  if (nextNodeToSync == 255) {
    return;
  }
  syncTimeout = Platform::system->localClock() + SYNC_TIMEOUT;

  // If we got here, it's time to sync the next node
  uint32_t observations[NUM_REQUESTS * 2];
  for (uint8_t i = 0; i < NUM_REQUESTS * 2; i++) {
    observations[i] = 0;
  }
  sendResponse(nextNodeToSync, 0, observations);
}

void parsePacket(uint8_t source) {
  debug("Parsing clock sync packet");
  uint8_t subPacketType = Platform::system->read8();
  Platform::system->read8(); // Reserved

  switch (subPacketType) {
  case CLOCK_SYNC_TYPE_P2P: {
    uint8_t observationNumber = Platform::system->read8();
    Platform::system->read8(); // reserved
    uint32_t observations[NUM_REQUESTS * 2];
    Platform::system->read(reinterpret_cast<uint8_t*>(observations),
        NUM_REQUESTS * 2 * sizeof(uint32_t));
    sendResponse(source, observationNumber, observations);
    break;
  }

  default: {
    error("Received unknown clock sync subpacket type %d", subPacketType);
    break;
  }
  }
}

} // namespace ProtocolClockSync

} // namespace rvl
