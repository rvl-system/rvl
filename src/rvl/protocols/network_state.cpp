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

#include "./rvl/protocols/network_state.hpp"
#include "./rvl/config.hpp"
#include "./rvl/platform.hpp"
#include <algorithm>
#include <string.h>

namespace rvl {

namespace NetworkState {

#define CONTROLLER_NODE_EXPIRATION_DURATION 10000
#define CLOCK_SYNC_MIN_INTERVAL 10000

uint32_t nodeTimestamps[NUM_NODES];
uint32_t nodeClockTimestamps[NUM_NODES];

uint8_t controllerNode = 255;
uint32_t controllerNodeLastRefreshed = 0;
uint32_t localClockLastRefreshed = 0;

void init() {
  memset(nodeTimestamps, 0, sizeof(uint32_t) * NUM_NODES);
  memset(nodeClockTimestamps, 0, sizeof(uint32_t) * NUM_NODES);
}

void loop() {
  int32_t expirationTime = std::max(0,
      static_cast<int32_t>(Platform::system->localClock()) -
          CONTROLLER_NODE_EXPIRATION_DURATION);
  for (uint8_t i = 0; i < NUM_NODES; i++) {
    if (nodeTimestamps[i] > 0 && nodeTimestamps[i] < expirationTime) {
      info("Node %d expired from the network map", i);
      nodeTimestamps[i] = 0;
      nodeClockTimestamps[i] = 0;
    }
  }
  bool synchronized = false;
  if (getDeviceMode() == DeviceMode::Controller) {
    synchronized = isClockSynchronizationActive();
  } else {
    synchronized = isClockSynchronizationActive() && isControllerActive();
  }
  if (synchronized != getSynchronizationState()) {
    setSynchronizationState(synchronized);
  }
}

void refreshNode(uint8_t node) {
  if (!isNodeActive(node)) {
    debug("Adding node %d to the network map", node);
  }
  nodeTimestamps[node] = Platform::system->localClock();
}

void refreshNodeClockSyncTime(uint8_t node) {
  debug("Finished updating clock for node %d", node);
  nodeClockTimestamps[node] = Platform::system->localClock();
}

uint8_t getNumNodes() {
  uint8_t numNodes = 0;
  for (uint8_t i = 0; i < NUM_NODES; i++) {
    if (isNodeActive(i)) {
      numNodes++;
    }
  }
  return numNodes;
}

bool isNodeActive(uint8_t node) {
  return nodeTimestamps[node] > 0;
}

bool isControllerNode(uint8_t node) {
  // First, we check if we're in controller mode, in which case we always ignore
  // remote control
  if (getDeviceMode() == DeviceMode::Controller) {
    return node == Platform::system->getDeviceId();
  }

  uint32_t currentTime = Platform::system->localClock();

  // Check if we've never seen a controller node before
  if (controllerNode == 255) {
    controllerNode = node;
    controllerNodeLastRefreshed = currentTime;
    return true;
  }

  // Check if this is the same controller node we've seen before, or not. If not
  // there *may* two active controllers at the same time, but it's also possible
  // the old one is no longer a controller
  if (controllerNode != node) {
    // Check if the old controller hasn't broadcast in a while, meaning it's
    // likely offline or no longer in controller mode and can be replaced with
    // this new controller
    if (currentTime - controllerNodeLastRefreshed >
        CONTROLLER_NODE_EXPIRATION_DURATION)
    {
      controllerNode = node;
      controllerNodeLastRefreshed = currentTime;
    }
  } else {
    controllerNodeLastRefreshed = currentTime;
  }
  return node == controllerNode;
}

bool isControllerActive() {
  return (controllerNodeLastRefreshed > 0) &&
      (Platform::system->localClock() - controllerNodeLastRefreshed <
          CONTROLLER_NODE_EXPIRATION_DURATION);
}

uint8_t getNextClockNode() {
  uint32_t now = Platform::system->localClock();
  uint32_t oldestClock = UINT32_MAX;
  uint8_t oldestNode = 255;
  for (uint8_t i = 0; i < NUM_NODES; i++) {
    if (nodeTimestamps[i] > 0 && nodeClockTimestamps[i] < oldestClock &&
        (nodeClockTimestamps[i] == 0 ||
            now - nodeClockTimestamps[i] > CLOCK_SYNC_MIN_INTERVAL))
    {
      oldestNode = i;
      oldestClock = nodeClockTimestamps[i];
    }
  }
  return oldestNode;
}

void refreshLocalClockSynchronization() {
  localClockLastRefreshed = Platform::system->localClock();
}

bool isClockSynchronizationActive() {
  return (localClockLastRefreshed > 0) &&
      (Platform::system->localClock() - localClockLastRefreshed <
          CONTROLLER_NODE_EXPIRATION_DURATION);
}

} // namespace NetworkState

} // namespace rvl
