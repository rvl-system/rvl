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
#include "./rvl/config.hpp"
#include "./rvl/platform.hpp"
#include "./rvl/protocols/network_state.hpp"
#include "./rvl/protocols/protocol.hpp"
#include "./rvl/protocols/system/system.hpp"
#include "./rvl/protocols/wave/wave.hpp"
#include <list>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace rvl {

#define MAX_PACKETS_PER_LOOP 32

void init(System* newSystem) {
  Platform::init(newSystem);
  Protocol::init();
}

void loop() {
  Platform::system->loop();
  // Run before isConnected to handle the case where we were connected, but then
  // got disconnected. When this happens, we have to make sure that timeouts are
  // still processed so they're not in an incorrect state when we reconnect.
  NetworkState::loop();

  if (!Platform::system->isConnected()) {
    return;
  }

  // Drain all pending packets, bounded so a packet storm can't starve the
  // rest of the loop
  for (uint8_t i = 0; i < MAX_PACKETS_PER_LOOP; i++) {
    int packetSize = Platform::system->parsePacket();
    if (packetSize == 0) {
      break;
    }
    uint8_t receivedSignature[4];
    Platform::system->read(receivedSignature, 4);
    if (memcmp(receivedSignature, rvl::signature, 4) == 0) {
      Protocol::parsePacket();
    } else {
      Platform::system->endRead();
    }
  }

  Protocol::loop();
}

void System::setConnectedState(bool connected) {
  setLinkUpState(connected);
}

bool System::isConnected() {
  return rvl::isConnected();
}

} // namespace rvl
