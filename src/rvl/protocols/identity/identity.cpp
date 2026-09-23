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

#include "./rvl/protocols/identity/identity.hpp"
#include "./rvl.hpp"
#include "./rvl/config.hpp"
#include "./rvl/platform.hpp"
#include "./rvl/protocols/infrastructure.hpp"
#include <stdint.h>

namespace rvl {

namespace ProtocolIdentity {

#define ID_RETRY_INTERVAL 1000
#define ID_RETRY_JITTER 250

uint32_t nextRequestTime;
bool requested = false;

void init() {
  nextRequestTime = Platform::system->localClock();
}

void sendRequest() {
  auto& infrastructure = Platform::system->infrastructure();
  ProtocolInfrastructure::beginCoordinatorWrite(RVLI_PACKET_TYPE_ID_ASSIGNMENT);
  infrastructure.write8(ID_REQUEST_TYPE);
  infrastructure.endWrite();
}

void loop() {
  uint32_t now = Platform::system->localClock();
  if (!getLinkUpState()) {
    // Pinned to now rather than zeroed, so the first loop after the link comes
    // up requests immediately. A zeroed timer would read as the future once
    // uptime passes 2^31 ms
    setDeviceId(UNASSIGNED_DEVICE_ID);
    nextRequestTime = now;
    requested = false;
    return;
  }
  if (getDeviceId() != UNASSIGNED_DEVICE_ID) {
    return;
  }
  if (static_cast<int32_t>(now - nextRequestTime) < 0) {
    return;
  }
  if (!requested) {
    info("Requesting device ID");
    requested = true;
  }
  sendRequest();
  // Jittered so boards that lost the same round don't retry in lockstep
  nextRequestTime =
      now + ID_RETRY_INTERVAL + Platform::system->random() % ID_RETRY_JITTER;
}

void parsePacket() {
  auto& infrastructure = Platform::system->infrastructure();
  // A request has no ID byte, so check the type before reading one
  if (infrastructure.read8() != ID_REPLY_TYPE) {
    return;
  }
  uint8_t id = infrastructure.read8();
  if (id >= NUM_DEVICE_IDS || id == getDeviceId()) {
    return;
  }
  info("Assigned device ID %d", id);
  setDeviceId(id);
}

} // namespace ProtocolIdentity

} // namespace rvl
