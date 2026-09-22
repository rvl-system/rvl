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
#include <stdint.h>
#include <string.h>

namespace rvl {

namespace ProtocolInfrastructure {

void parsePacket() {
  auto& infrastructure = Platform::system->infrastructure();

  uint8_t signature[4];
  infrastructure.read(signature, 4);
  if (memcmp(signature, rvliSignature, 4) != 0) {
    infrastructure.endRead();
    return;
  }

  uint8_t version = infrastructure.read8();
  if (version != PROTOCOL_VERSION) {
    error("Received unsupported RVLI protocol version %d, ignoring", version);
    infrastructure.endRead();
    return;
  }

  uint8_t source = infrastructure.read8();
  uint8_t packetType = infrastructure.read8();
  infrastructure.read8(); // reserved

  // Checked before the self filter: a node with no ID is 255 too, and an ID
  // request from a peer must be dropped for being a request, not for looking
  // like our own
  if (source >= NUM_DEVICE_IDS) {
    infrastructure.endRead();
    return;
  }

  // Ignore our own packets
  if (source == getDeviceId()) {
    infrastructure.endRead();
    return;
  }

  switch (packetType) {
  default:
    error("Received unknown RVLI packet type %d", packetType);
    break;
  }
  infrastructure.endRead();
}

} // namespace ProtocolInfrastructure

} // namespace rvl
