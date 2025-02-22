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
#include "./rvl/protocols/protocol.hpp"
#include "./rvl/protocols/system/system.hpp"
#include "./rvl/protocols/wave/wave.hpp"
#include <list>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace rvl {

void init(System* newSystem) {
  Platform::init(newSystem);
  Protocol::init();
}

void loop() {
  Platform::system->loop();
  stateLoop();
  if (!Platform::system->isConnected()) {
    return;
  }

  int packetSize = Platform::system->parsePacket();
  if (packetSize != 0) {
    uint8_t receivedSignature[4];
    Platform::system->read(receivedSignature, 4);
    if (memcmp(receivedSignature, rvl::signature, 4) == 0) {
      Protocol::parsePacket();
    }
  }

  Protocol::loop();
}

bool rvlConnectedState = false;

void System::setConnectedState(bool connected) {
  if (rvlConnectedState != connected) {
    rvlConnectedState = connected;
    emit(EVENT_CONNECTION_STATE_CHANGED);
  }
}

bool System::isConnected() {
  return rvlConnectedState;
}

} // namespace rvl
