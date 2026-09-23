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

#include "./rvl/protocols/infrastructure.hpp"
#include "./rvl.hpp"
#include "./rvl/config.hpp"
#include "./rvl/platform.hpp"
#include "./rvl/protocols/clock_sync/clock_sync.hpp"
#include "./rvl/protocols/header.hpp"
#include "./rvl/protocols/identity/identity.hpp"
#include <stdint.h>

namespace rvl {

namespace ProtocolInfrastructure {

void init() {
  ProtocolIdentity::init();
  ProtocolClockSync::init();
}

void loop() {
  ProtocolIdentity::loop();
}

void parsePacket() {
  auto& infrastructure = Platform::system->infrastructure();

  uint8_t source;
  uint8_t packetType;
  if (!readHeader(infrastructure, rvliSignature, source, packetType)) {
    return;
  }
  infrastructure.read8(); // reserved

  switch (packetType) {
  case RVLI_PACKET_TYPE_ID_ASSIGNMENT:
    ProtocolIdentity::parsePacket();
    break;
  case RVLI_PACKET_TYPE_CLOCK_SYNC:
    // Clock sync stores this node's observations under its own ID
    if (isConnected()) {
      ProtocolClockSync::parsePacket(source);
    }
    break;
  default:
    error("Received unknown RVLI packet type %d", packetType);
    break;
  }
  infrastructure.endRead();
}

void writeHeader(uint8_t packetType) {
  auto& infrastructure = Platform::system->infrastructure();
  infrastructure.write(rvliSignature, 4);
  infrastructure.write8(PROTOCOL_VERSION);
  infrastructure.write8(getDeviceId());
  infrastructure.write8(packetType);
  infrastructure.write8(0);
}

void beginBroadcastWrite(uint8_t packetType) {
  Platform::system->infrastructure().beginBroadcastWrite();
  writeHeader(packetType);
}

void beginCoordinatorWrite(uint8_t packetType) {
  Platform::system->infrastructure().beginCoordinatorWrite();
  writeHeader(packetType);
}

} // namespace ProtocolInfrastructure

} // namespace rvl
