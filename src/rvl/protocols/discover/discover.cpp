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

#include <limits.h>
#include <stdint.h>
#include "./rvl.h"
#include "./rvl/protocols/protocol.h"
#include "./rvl/protocols/discover/discover.h"
#include "./rvl/protocols/network_state.h"
#include "./rvl/platform.h"
#include "./rvl/config.h"

namespace ProtocolDiscover {

#define SYNC_ITERATION_MODULO 10
bool hasSyncedThisLoop = false;

void sync();

/*
Type: 1 byte = 1: ping, 2 pong
Reserved: 3 bytes
*/

void init() {
}

void loop() {
  if (Platform::platform->getDeviceMode() != RVLDeviceMode::Controller) {
    return;
  }
  if (Platform::platform->getLocalTime() % CLIENT_SYNC_INTERVAL < SYNC_ITERATION_MODULO) {
    hasSyncedThisLoop = false;
    return;
  }
  if (hasSyncedThisLoop) {
    return;
  }
  hasSyncedThisLoop = true;
  sync();
}

void sync() {
  Platform::logging->debug("Broadcasting discover packet");
  Platform::transport->beginWrite();
  Protocol::sendBroadcastHeader(PACKET_TYPE_DISCOVER);
  Platform::transport->write8(1);  // type = ping
  Platform::transport->write8(0);  // reserved
  Platform::transport->write16(0);  // reserved
  Platform::transport->endWrite();
}

void parsePacket(uint8_t source) {
  Platform::logging->debug("Parsing Discover packet");
  uint8_t type = Platform::transport->read8();
  Platform::transport->read8();  // reserved
  Platform::transport->read16();  // reserved
  NetworkState::refreshNode(source);
  if (type == 1) {
    Platform::transport->beginWrite();
    Protocol::sendMulticastHeader(PACKET_TYPE_DISCOVER);
    Platform::transport->write8(2);  // type = pong
    Platform::transport->write8(0);  // reserved
    Platform::transport->write16(0);  // reserved
    Platform::transport->endWrite();
  }
}

}  // namespace ProtocolDiscover
