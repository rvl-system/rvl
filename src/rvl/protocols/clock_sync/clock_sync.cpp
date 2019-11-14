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

#define CLOCK_SYNC_SUBPACKET_TYPE_START 1
#define CLOCK_SYNC_SUBPACKET_TYPE_REQUEST 2
#define CLOCK_SYNC_SUBPACKET_TYPE_RESPONSE 3

#define NUM_REQUESTS 3

#define SYNC_ITERATION_MODULO 500
#define SYNC_ITERATION_MODULO_MAX 1400

#define SYNC_TIMEOUT SYNC_ITERATION_MODULO_MAX - SYNC_ITERATION_MODULO

uint16_t syncId = 0;
uint8_t currentSyncNode = 255;
uint32_t syncTimeout = 0;

uint32_t observedRequestTimes[NUM_REQUESTS];
uint32_t remoteTimes[NUM_REQUESTS];
uint32_t observedResponseTimes[NUM_REQUESTS];
uint32_t processTime = UINT32_MAX;
uint16_t currentObservation = 0;

/*
Note: the order of packets defined here are listed in algorithmic order

Parent packet:
Type: 1 byte = 1: Start, 2: Request, 3: Response
ID: 2 bytes = A counter so that packets can be associated with each other, not currently used
Reserved: 1 byte

Start Packet Type (controller->receiver):
No body

Request Packet Type (receiver->controller):
Observation: 1 byte = The observation number, between 1 and NUM_REQUESTS
Reserved: 1 byte

Response Packet Type (controller->receiver):
clock: 4 bytes = the clock of the root node
*/

// TODO(nebrius): add support for sync ID checking

void init() {
}

void synchronizeNextNode() {
  // Make sure we are in our alloted network usage time, and if not, skip for now
  uint32_t localTime = Platform::platform->getLocalTime();
  uint32_t syncTime = localTime % CLIENT_SYNC_INTERVAL;
  if (syncTime < SYNC_ITERATION_MODULO || syncTime > SYNC_ITERATION_MODULO_MAX) {
    return;
  }

  // Check if there is a current sync in progress, and if so skip
  if (syncTimeout > 0) {
    // Check if the sync has timed out and we need to move to the next one,
    // or let this sync continue doing its thing
    if (Platform::platform->getLocalTime() > syncTimeout) {
      Platform::logging->debug("Clock synchronization with node %d has timed out", currentSyncNode);
      syncTimeout = 0;
    } else {
      return;
    }
  }

  // First we check if the next node has value 255 (broadcast) which means there
  // are no other known nodes in this system, and we can stop here
  uint8_t nextSyncNode = NetworkState::getNextNode(currentSyncNode);
  if (nextSyncNode == 255) {
    return;
  }

  // If we got here, then this ia valid next sync node
  currentSyncNode = nextSyncNode;
  Platform::logging->debug("Starting clock synchronization with node %d", currentSyncNode);

  // Start the timeout. If the two nodes don't finish exchanging information
  // within this time, we skip and go to the next one.
  syncTimeout = Platform::platform->getLocalTime() + SYNC_TIMEOUT;

  // Send the Start packet
  Platform::transport->beginWrite();
  Protocol::sendHeader(PACKET_TYPE_CLOCK_SYNC, currentSyncNode);
  Platform::transport->write8(CLOCK_SYNC_SUBPACKET_TYPE_START);
  Platform::transport->write16(syncId++);
  Platform::transport->write8(0);  // reserved
  Platform::transport->endWrite();
}

void loop() {
  if (Platform::platform->getDeviceMode() != RVLDeviceMode::Controller) {
    return;
  }
  synchronizeNextNode();
}

void processObservations() {
  int32_t offsets[NUM_REQUESTS];
  for (uint8_t i = 0; i < NUM_REQUESTS; i++) {
    Platform::logging->debug("Times: request=%d remote=%d response=%d", observedRequestTimes[i], observedResponseTimes[i], remoteTimes[i]);
    uint32_t delay = (observedResponseTimes[i] - observedRequestTimes[i]) / 2;
    uint32_t correctedRemoteTime = remoteTimes[i] - delay;
    offsets[i] = correctedRemoteTime - observedRequestTimes[i];
  }
  std::sort(std::begin(offsets), std::end(offsets));
  uint32_t medianOffset = offsets[NUM_REQUESTS / 2];
  Platform::logging->debug("Updating animation clock with offset=%d, synchronization took %dms",
    medianOffset, observedResponseTimes[NUM_REQUESTS - 1] - observedRequestTimes[1]);
  Platform::platform->setAnimationClock(Platform::platform->getAnimationClock() + medianOffset);
  NetworkState::refreshClockSynchronization();
}

void sendRequestPacket(uint8_t source, uint16_t id) {
  uint32_t observedTime = Platform::platform->getAnimationClock();
  observedRequestTimes[currentObservation] = observedTime;
  Platform::transport->beginWrite();
  Protocol::sendHeader(PACKET_TYPE_CLOCK_SYNC, source);
  Platform::transport->write8(CLOCK_SYNC_SUBPACKET_TYPE_REQUEST);
  Platform::transport->write16(id);
  Platform::transport->write8(0);  // reserved
  Platform::transport->write8(currentObservation);
  Platform::transport->write8(0);  // reserved
  Platform::transport->endWrite();
  Platform::logging->debug("Sent clock sync observation request at time %d", observedTime);
}

void parsePacket(uint8_t source) {
  uint8_t subPacketType = Platform::transport->read8();
  uint16_t id = Platform::transport->read16();
  Platform::transport->read8();  // reserved

  switch (subPacketType) {
    case CLOCK_SYNC_SUBPACKET_TYPE_START: {
      currentObservation = 0;
      memset(observedRequestTimes, 0, sizeof(observedRequestTimes));
      memset(remoteTimes, 0, sizeof(remoteTimes));
      memset(observedResponseTimes, 0, sizeof(observedResponseTimes));
      sendRequestPacket(source, id);
      break;
    }
    case CLOCK_SYNC_SUBPACKET_TYPE_REQUEST: {
      uint32_t observedTime = Platform::platform->getAnimationClock();
      Platform::transport->beginWrite();
      Protocol::sendHeader(PACKET_TYPE_CLOCK_SYNC, source);
      Platform::transport->write8(CLOCK_SYNC_SUBPACKET_TYPE_RESPONSE);
      Platform::transport->write16(id);
      Platform::transport->write8(0);  // reserved
      Platform::transport->write32(observedTime);
      Platform::transport->endWrite();
      Platform::logging->debug("Responded to clock sync request from %d with observed time %d", source, observedTime);
      uint8_t remoteObservation = Platform::transport->read8();
      if (remoteObservation == NUM_REQUESTS) {
        syncTimeout = 0;
      }
      break;
    }
    case CLOCK_SYNC_SUBPACKET_TYPE_RESPONSE: {
      uint32_t observedTime = Platform::platform->getAnimationClock();
      uint32_t remoteTime = Platform::transport->read32();
      observedResponseTimes[currentObservation] = observedTime;
      remoteTimes[currentObservation] = remoteTime;
      currentObservation++;
      if (currentObservation == NUM_REQUESTS) {
        processObservations();
      } else {
        sendRequestPacket(source, id);
      }
      break;
    }

    default: {
      Platform::logging->error("Received unknown clock sync subpacket type %d", subPacketType);
      break;
    }
  }
}

}  // namespace ProtocolClockSync
