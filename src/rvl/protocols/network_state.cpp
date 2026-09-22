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
#include "./rvl/platform.hpp"

namespace rvl {

namespace NetworkState {

#define CONTROLLER_NODE_EXPIRATION_DURATION 10000

// Configured to ensure a free-running clock derived from a mediocre crystal
// won't drift more than the accuracy of the clock sync algorithm. A
// 20ppm crystal drift accumulates ~1.2ms of error in 60s.
#define CLOCK_SYNC_EXPIRATION_DURATION 60000

// Stores the ID of the controller. 255 indicates we haven't communicated with a
// controller node yet, since 255 is never a valid ID.
uint8_t controllerNode = 255;
uint32_t controllerNodeLastRefreshed = 0;
uint32_t localClockLastRefreshed = 0;

void onChannelUpdated() {
  // If we previously connected to a controller on the old channel, we need to
  // reset it since by definition controllers are scoped to a single channel
  controllerNode = 255;
  controllerNodeLastRefreshed = 0;
}

void init() {
  on(EVENT_CHANNEL_UPDATED, onChannelUpdated);
}

void loop() {
  setClockSyncedState(isClockSynchronizationActive());
  setControllerActiveState(isControllerActive());
}

bool isControllerNode(uint8_t node) {
  // First, we check if we're in controller mode, in which case we always ignore
  // remote control
  if (getDeviceMode() == DeviceMode::Controller) {
    return node == getDeviceId();
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

void refreshLocalClockSynchronization() {
  localClockLastRefreshed = Platform::system->localClock();
}

bool isClockSynchronizationActive() {
  return (localClockLastRefreshed > 0) &&
      (Platform::system->localClock() - localClockLastRefreshed <
          CLOCK_SYNC_EXPIRATION_DURATION);
}

} // namespace NetworkState

} // namespace rvl
