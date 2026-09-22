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
#include "./rvl/protocols/animation.hpp"
#include "./rvl/protocols/network_state.hpp"
#include <algorithm>
#include <stdint.h>
#include <string.h>

namespace rvl {

namespace ProtocolClockSync {

#define CLOCK_SYNC_PACKET_TYPE_REFERENCE_BROADCAST 1
#define CLOCK_SYNC_PACKET_TYPE_OBSERVATION 2

uint32_t observations[NUM_OBSERVATIONS_IN_SET][NUM_DEVICE_IDS];
uint16_t observationIds[NUM_OBSERVATIONS_IN_SET];

/*
Reference Broadcast Synchronization, an implementation of the algorith
described in "Fine-Grained Network Time Synchronization using Reference
Broadcasts" by Jeremy Elson, Lewis Girod and Deborah Estrin

Note: the order of packets defined here are listed in algorithmic order

Parent packet:
Type: 1 byte = 1: Reference broadcast, 2: Observation
ID: 2 bytes = the id of the synchronization set
Reserved: 1 byte

Reference broadcast packet:
isStartOfSet: 1 byte = 1 if this is the first broadcast in the reference set,
  otherwise 0
Reserved: 1 byte

Observation packet:
clock: 4 bytes = the observed time of the reference
*/

// Each observation row holds data for a single reference broadcast id. If a
// row still holds data from an older broadcast (a set that never completed),
// clear it before reuse so observations of different broadcasts never mix
// within one row
void prepareObservationRow(uint16_t broadcastId) {
  uint8_t row = broadcastId % NUM_OBSERVATIONS_IN_SET;
  if (observationIds[row] != broadcastId) {
    memset(observations[row], 0, sizeof(observations[row]));
    observationIds[row] = broadcastId;
  }
}

void processObservations() {
  int64_t offsetSum = 0;
  uint8_t localNode = getDeviceId();
  if (localNode >= NUM_DEVICE_IDS) {
    return;
  }
  uint8_t numObservationsProcessed = 0;
  uint8_t numNodesObserved = 0;
  for (uint8_t observation = 0; observation < NUM_OBSERVATIONS_IN_SET;
      observation++)
  {
    uint32_t localObservedTime = observations[observation][localNode];

    // If we never observed this reference broadcast ourselves (it was lost),
    // we have nothing to compare the other nodes' observations against, so
    // discard this row. Without this check, localObservedTime would be 0 and
    // the computed offset would jump the clock by the entire median value
    if (localObservedTime == 0) {
      memset(observations[observation], 0, sizeof(observations[observation]));
      continue;
    }

    // Sort the array so we can get the median time
    std::sort(
        observations[observation], observations[observation] + NUM_DEVICE_IDS);
    uint8_t head = 0;
    while (head < NUM_DEVICE_IDS) {
      if (observations[observation][head] > 0) {
        break;
      } else {
        head++;
      }
    }

    // On rare occasions, we end up with an empty observation set. This is a bug
    // that should be fixed, but let's also act defensively so we don't get a
    // divide by 0 segfault and take the entire system down
    if (head == NUM_DEVICE_IDS) {
      continue;
    }

    // Every node that observed this reference contributed one entry, so the
    // widest row is the number of nodes that heard the same broadcast
    if (NUM_DEVICE_IDS - head > numNodesObserved) {
      numNodesObserved = NUM_DEVICE_IDS - head;
    }

    // Calculate the offset for this observation. The subtraction is unsigned so
    // that it wraps to the correct signed delta however far apart the clocks
    // are, and the sum is 64-bit so a cold start's near-2^31 deltas can't
    // overflow it
    uint32_t medianObservedTime =
        observations[observation][head + (NUM_DEVICE_IDS - head) / 2];
    offsetSum += static_cast<int32_t>(medianObservedTime - localObservedTime);
    numObservationsProcessed++;

    // Reset the observation array for reuse next time
    for (uint8_t node = head; node < NUM_DEVICE_IDS; node++) {
      observations[observation][node] = 0;
    }
  }

  // Calculate the average offset for all observations
  if (numObservationsProcessed > 0) {
    int32_t averageOffset =
        static_cast<int32_t>(offsetSum / numObservationsProcessed);
    debug("Updating animation clock with offset: %d from %d nodes across %d "
          "references",
        averageOffset, numNodesObserved, numObservationsProcessed);
    adjustAnimationClock(averageOffset);
    NetworkState::refreshLocalClockSynchronization();
  }
}

void init() {
  memset(observations, 0, sizeof(observations));
  memset(observationIds, 0, sizeof(observationIds));
}

void parsePacket(uint8_t source) {
  auto& animation = Platform::system->animation();
  uint8_t packetType = animation.read8();
  uint16_t id = animation.read16();
  animation.read8(); // Reserved

  switch (packetType) {
  case CLOCK_SYNC_PACKET_TYPE_REFERENCE_BROADCAST: {
    // Must run before the observed time is computed below: processing changes
    // the clock offset, and a time computed with the old one would be stored as
    // a stale entry in the new set
    uint8_t isStartOfSet = animation.read8();
    animation.read8(); // reserved
    if (isStartOfSet == 1) {
      processObservations();
    }

    // Convert the packet's arrival time, not the animation clock cached at the
    // top of the loop: the cached value is stale by however long ago the loop
    // tick started, and that error varies per node
    uint32_t observedTime = toAnimationClock(animation.packetArrivalTime());

    // Send the observed time out to everyone
    debug("Received reference broadcast with id %d at observed time %d", id,
        observedTime);
    ProtocolAnimation::beginBroadcastWrite(PACKET_TYPE_CLOCK_SYNC);
    animation.write8(CLOCK_SYNC_PACKET_TYPE_OBSERVATION);
    animation.write16(id);
    animation.write8(0); // Reserved
    animation.write32(observedTime);
    animation.endWrite();

    // Store this node in the observation list so that all nodes have the same
    // observation table. This would normally be missing, since we wouldn't
    // receive the message that was just sent
    uint8_t localNode = getDeviceId();
    if (localNode >= NUM_DEVICE_IDS) {
      break;
    }
    prepareObservationRow(id);
    uint8_t observationStep = id % NUM_OBSERVATIONS_IN_SET;
    observations[observationStep][localNode] = observedTime;
    break;
  }

  case CLOCK_SYNC_PACKET_TYPE_OBSERVATION: {
    uint32_t clock = animation.read32();
    prepareObservationRow(id);
    uint8_t observationStep = id % NUM_OBSERVATIONS_IN_SET;
    observations[observationStep][source] = clock;
    break;
  }

  default: {
    debug("Received unknown clock sync subpacket type %d", packetType);
    break;
  }
  }
}

} // namespace ProtocolClockSync

} // namespace rvl
