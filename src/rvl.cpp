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
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <list>
#include "./rvl.h"
#include "./rvl/config.h"
#include "./rvl/platform.h"
#include "./rvl/protocols/protocol.h"
#include "./rvl/protocols/wave/wave.h"
#include "./rvl/protocols/system/system.h"

void RVLInit(RVLTransport* newTransport) {
  rvl::Platform::init(newTransport);
  rvl::Protocol::init();
}

void RVLLoop() {
  rvl::Platform::transport->loop();
  rvl::stateLoop();
  if (!rvl::Platform::transport->isConnected()) {
    return;
  }

  int packetSize = rvl::Platform::transport->parsePacket();
  if (packetSize != 0) {
    uint8_t receivedSignature[4];
    rvl::Platform::transport->read(receivedSignature, 4);
    if (memcmp(receivedSignature, rvl::signature, 4) == 0) {
      rvl::Protocol::parsePacket();
    }
  }

  rvl::Protocol::loop();
}

bool rvlConnectedState = false;

void RVLTransport::setConnectedState(bool connected) {
  if (rvlConnectedState != connected) {
    rvlConnectedState = connected;
    rvl::emit(EVENT_CONNECTION_STATE_CHANGED);
  }
}

bool RVLTransport::isConnected() {
  return rvlConnectedState;
}
