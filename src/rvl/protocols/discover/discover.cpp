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

#define DISCOVER_SUBPACKET_TYPE_PING 1
#define DISCOVER_SUBPACKET_TYPE_PONG 2

#define SYNC_ITERATION_MODULO 10

bool hasSyncedThisLoop = false;

/*
Parent packet:
Type: 1 byte = 1: ping, 2 pong
Reserved: 1 byte

Ping Packet Type (controller->receiver):
No body

Pong Packet Type (receiver->controller):
No body
*/

void init() {
}

void loop() {
  if (Platform::platform->getDeviceMode() != RVLDeviceMode::Controller) {
    return;
  }
  if (millis() % CLIENT_SYNC_INTERVAL < SYNC_ITERATION_MODULO) {
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
  if (!Platform::transport->isConnected()) {
    return;
  }
  rvl::debug("Multicasting discover packet");
  Platform::transport->beginWrite(Protocol::getMulticastAddress());
  Protocol::sendMulticastHeader(PACKET_TYPE_DISCOVER);
  Platform::transport->write8(DISCOVER_SUBPACKET_TYPE_PING);
  Platform::transport->write8(0);  // reserved
  Platform::transport->endWrite();
}

void parsePacket(uint8_t source) {
  rvl::debug("Parsing Discover packet");

  uint8_t subPacketType = Platform::transport->read8();
  Platform::transport->read8();  // reserved
  NetworkState::refreshNode(source);

  switch (subPacketType) {
    case DISCOVER_SUBPACKET_TYPE_PING: {
      Platform::transport->beginWrite(Protocol::getMulticastAddress());
      Protocol::sendMulticastHeader(PACKET_TYPE_DISCOVER);
      Platform::transport->write8(DISCOVER_SUBPACKET_TYPE_PONG);
      Platform::transport->write8(0);  // reserved
      Platform::transport->endWrite();
      break;
    }

    case DISCOVER_SUBPACKET_TYPE_PONG: {
     // Don't need to do anything special here, since we already refreshed the node
      break;
    }

    default: {
      rvl::error("Received unknown clock sync subpacket type %d", subPacketType);
      break;
    }
  }
}

}  // namespace ProtocolDiscover
