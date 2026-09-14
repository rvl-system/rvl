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
#include <stdint.h>
#include <string.h>

namespace rvl {

namespace ProtocolClockSync {

#define CLOCK_SYNC_PACKET_TYPE_REFERENCE_BROADCAST 1
#define CLOCK_SYNC_PACKET_TYPE_OBSERVATION 2
#define NUM_OBSERVATIONS_IN_SET 3
#define NUM_NODES 240

#define SYNC_ITERATION_MODULO 500
#define SYNC_ITERATION_MODULO_MAX 1400

uint32_t observations[NUM_OBSERVATIONS_IN_SET][NUM_NODES];
uint16_t observationIds[NUM_OBSERVATIONS_IN_SET];
uint16_t numObservations = 0;
uint16_t id = 0;

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
  int32_t averageOffset = 0;
  uint8_t localNode = Platform::system->getDeviceId();
  uint8_t numObservationsProcessed = 0;
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
    uint32_t medianObservedTime =
        observations[observation][head + (NUM_NODES - head) / 2];
    averageOffset += static_cast<int32_t>(medianObservedTime) -
        static_cast<int32_t>(localObservedTime);
    numObservationsProcessed++;

    // Reset the observation array for reuse next time
    for (uint8_t node = head; node < NUM_NODES; node++) {
      observations[observation][node] = 0;
    }
  }

  // Calculate the average offset for all observations
  if (numObservationsProcessed > 0) {
    averageOffset /= numObservationsProcessed;
    debug("Updating animation clock with offset: %d", averageOffset);
    setAnimationClock(getAnimationClock() + averageOffset);
    NetworkState::refreshLocalClockSynchronization();
  }
}

void init() {
  memset(observations, 0, sizeof(observations));
  memset(observationIds, 0, sizeof(observationIds));
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

  // TODO: if we were to implement broadcast here, it would look like:
  /*
  Protocol::beginBroadcastWrite(PACKET_TYPE_CLOCK_SYNC);
  Platform::system->write8(CLOCK_SYNC_PACKET_TYPE_REFERENCE_BROADCAST);
  Platform::system->write16(id++);
  Platform::system->write8(0); // Reserved
  */
}

void parsePacket(uint8_t source) {
  uint8_t packetType = Platform::system->read8();
  uint16_t id = Platform::system->read16();
  Platform::system->read8(); // Reserved

  switch (packetType) {
  case CLOCK_SYNC_PACKET_TYPE_REFERENCE_BROADCAST: {
    // Check if this the start of a set, and we need to reset our observation
    // counter. This must happen before the observed time is computed below:
    // salvaging can change the clock offset, and an observed time computed
    // with the old offset would be stored as a stale entry in the new set,
    // producing a correction of (offset change / NUM_OBSERVATIONS_IN_SET) on
    // the next processing pass
    bool isStartOfSet = Platform::system->read8();
    Platform::system->read8(); // reserved
    if (isStartOfSet == 1) {
      // If the previous set never completed because packets were lost, salvage
      // whatever observations we did collect instead of letting them go stale
      if (numObservations > 0) {
        processObservations();
      }
      numObservations = 0;
    }

    // Convert the packet's arrival time, not the animation clock cached at the
    // top of the loop: the cached value is stale by however long ago the loop
    // tick started, and that error varies per node
    uint32_t observedTime =
        toAnimationClock(Platform::system->packetArrivalTime());

    // Send the observed time out to everyone
    debug("Received reference broadcast with id %d at observed time %d", id,
        observedTime);
    Protocol::beginBroadcastWrite(PACKET_TYPE_CLOCK_SYNC);
    Platform::system->write8(CLOCK_SYNC_PACKET_TYPE_OBSERVATION);
    Platform::system->write16(id);
    Platform::system->write8(0); // Reserved
    Platform::system->write32(observedTime);
    Platform::system->endWrite();

    // Store this node in the observation list so that all nodes have the same
    // observation table. This would normally be missing, since we wouldn't
    // receive the message that was just sent
    prepareObservationRow(id);
    uint8_t observationStep = id % NUM_OBSERVATIONS_IN_SET;
    observations[observationStep][Platform::system->getDeviceId()] =
        observedTime;
    break;
  }

  case CLOCK_SYNC_PACKET_TYPE_OBSERVATION: {
    uint32_t clock = Platform::system->read32();
    debug("Received observation from %d with clock %d", source, clock);
    prepareObservationRow(id);
    uint8_t observationStep = id % NUM_OBSERVATIONS_IN_SET;
    observations[observationStep][source] = clock;
    numObservations++;
    // On a lossy network the count can step over the expected total (duplicate
    // packets, nodes joining mid-set), so this must not require exact equality
    if (numObservations >=
        NUM_OBSERVATIONS_IN_SET * (NetworkState::getNumNodes() - 1))
    {
      processObservations();
      numObservations = 0;
    }
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
