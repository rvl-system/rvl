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

#include "./rvl.hpp"
#include "./rvl/platform.hpp"
#include "./rvl/protocols/animation.hpp"
#include "./rvl/protocols/infrastructure.hpp"
#include "./rvl/protocols/network_state.hpp"
#include <stdint.h>

namespace rvl {

#define MAX_PACKETS_PER_LOOP 32

void init(System* newSystem) {
  Platform::init(newSystem);
  NetworkState::init();
  ProtocolAnimation::init();
}

// Bounded, so a packet storm can't starve the rest of the loop. A template
// because the two protocols' endpoints share no base class
template <class Endpoint> void drain(Endpoint& endpoint, void (*dispatch)()) {
  for (uint8_t i = 0; i < MAX_PACKETS_PER_LOOP; i++) {
    if (endpoint.parsePacket() == 0) {
      return;
    }
    dispatch();
  }
}

void loop() {
  Platform::system->loop();
  // Run before anything that can bail on the connection state, so timeouts keep
  // decaying while we're disconnected and are correct when we reconnect
  NetworkState::loop();

  // Both protocols drain every iteration, connected or not, so nothing queues
  // up stale while we wait. Each dispatcher decides what it can accept
  drain(Platform::system->infrastructure(),
      ProtocolInfrastructure::parsePacket);
  drain(Platform::system->animation(), ProtocolAnimation::parsePacket);

  ProtocolAnimation::loop();
}

} // namespace rvl
