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

#include <stdint.h>
#include <algorithm>
#include "./rvl.h"
#include "./rvl/config.h"
#include "./rvl/platform.h"
#include "./rvl/protocols/protocol.h"
#include "./rvl/protocols/network_state.h"
#include "./rvl/protocols/clock_sync/clock_sync.h"

namespace ProtocolClockSync {

#define CLOCK_SYNC_PACKET_TYPE_REFERENCE_BROADCAST 1
#define CLOCK_SYNC_PACKET_TYPE_OBSERVATION 2
#define NUM_OBSERVATIONS_IN_SET 3
#define NUM_NODES 240

uint32_t observations[NUM_OBSERVATIONS_IN_SET][NUM_NODES];
uint32_t processTime = UINT32_MAX;
uint16_t numObservations = 0;

/*
Note: the order of packets defined here are listed in algorithmic order

Parent packet:
Type: 1 byte = 1: Reference broadcast, 2: Observation
ID: 2 bytes = the id of the synchronization set
Reserved: 1 byte

Reference broadcast packet:
isStartOfSet: 1 byte = 1 if this is the first broadcast in the reference set, otherwise 0
Reserved: 1 byte

Observation packet:
clock: 4 bytes = the observed time of the reference
*/

void processObservations() {
  int32_t averageOffset = 0;
  uint8_t localNode = Platform::platform->getDeviceId();
  uint8_t numObservationsProcessed = 0;
  for (uint8_t observation = 0; observation < NUM_OBSERVATIONS_IN_SET; observation++) {
    uint32_t localObservedTime = observations[observation][localNode];

    // Sort the array so we can get the median time
    std::sort(observations[observation], observations[observation] + NUM_NODES);
    uint8_t head = 0;
    while (head < NUM_NODES) {
      if (observations[observation][head] > 0) {
        break;
      } else {
        head++;
      }
    }

    // On rare occasions, we end up with an empty observation set. This is a bug
    // that should be fixed, but let's also act defensively so we don't get a
    // divide by 0 segfault and take the entire system down
    if (head == NUM_NODES) {
      continue;
    }

    // Calculate the offset for this observation
    uint32_t medianObservedTime = observations[observation][head + (NUM_NODES - head) / 2];
    averageOffset += static_cast<int32_t>(medianObservedTime) - static_cast<int32_t>(localObservedTime);
    numObservationsProcessed++;

    // Reset the observation array for reuse next time
    for (uint8_t node = head; node < NUM_NODES; node++) {
      observations[observation][node] = 0;
    }
  }

  // Calculate the average offset for all observations
  if (numObservationsProcessed > 0) {
    averageOffset /= numObservationsProcessed;
    Platform::logging->debug("Updating animation clock with offset: %d", averageOffset);
    Platform::platform->setAnimationClock(Platform::platform->getAnimationClock() + averageOffset);
    NetworkState::refreshClockSynchronization();
  }
}

void init() {
  memset(observations, 0, sizeof(observations));
}

void loop() {
}

void parsePacket(uint8_t source) {
  uint8_t packetType = Platform::transport->read8();
  uint16_t id = Platform::transport->read16();
  Platform::transport->read8();  // reserved

  switch (packetType) {
    case CLOCK_SYNC_PACKET_TYPE_REFERENCE_BROADCAST: {
      uint32_t observedTime = Platform::platform->getAnimationClock();

      // Send the observed time out to everyone
      Platform::logging->debug("Received reference broadcast with id %d at observed time %d", id, observedTime);
      Platform::transport->beginWrite();
      Protocol::sendBroadcastHeader(PACKET_TYPE_CLOCK_SYNC);
      Platform::transport->write8(CLOCK_SYNC_PACKET_TYPE_OBSERVATION);
      Platform::transport->write16(id);
      Platform::transport->write8(0);  // reserved
      Platform::transport->write32(observedTime);
      Platform::transport->endWrite();

      // Check if this the start of a set, and we need to reset our observation counter
      bool isStartOfSet = Platform::transport->read8();
      Platform::transport->read8();  // reserved
      if (isStartOfSet == 1) {
        numObservations = 0;
      }

      // Store this node in the observation list so that all nodes have the same observation
      // table. This would normally be missing, since we wouldn't receive the message that
      // was just sent
      uint8_t observationStep = id % NUM_OBSERVATIONS_IN_SET;
      observations[observationStep][Platform::platform->getDeviceId()] = observedTime;
      break;
    }

    case CLOCK_SYNC_PACKET_TYPE_OBSERVATION: {
      uint32_t clock = Platform::transport->read32();
      Platform::logging->debug("Received observation from %d with clock %d", source, clock);
      uint8_t observationStep = id % NUM_OBSERVATIONS_IN_SET;
      observations[observationStep][source] = clock;
      numObservations++;
      if (numObservations == NUM_OBSERVATIONS_IN_SET * (NetworkState::getNumNodes() - 1)) {
        processObservations();
      }
      break;
    }

    default: {
      Platform::logging->debug("Received unknown clock sync subpacket type %d", packetType);
      break;
    }
  }
}

}  // namespace ProtocolClockSync
