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

#ifndef RVL_PROTOCOLS_HEADER_H_
#define RVL_PROTOCOLS_HEADER_H_

#include "./rvl.hpp"
#include "./rvl/config.hpp"
#include <stdint.h>
#include <string.h>

namespace rvl {

// Reads the prefix both protocols' headers share: signature, version, source
// and packet type. Returns false, with the read ended, for a packet the
// dispatcher must not see. A template because the two protocols' endpoints
// share no base class
template <class Endpoint>
bool readHeader(Endpoint& endpoint, const uint8_t* signature, uint8_t& source,
    uint8_t& packetType) {
  uint8_t packetSignature[4];
  endpoint.read(packetSignature, 4);
  if (memcmp(packetSignature, signature, 4) != 0) {
    endpoint.endRead();
    return false;
  }

  uint8_t version = endpoint.read8();
  if (version != PROTOCOL_VERSION) {
    error("Received unsupported %.4s protocol version %d, ignoring",
        reinterpret_cast<const char*>(signature), version);
    endpoint.endRead();
    return false;
  }

  source = endpoint.read8();
  packetType = endpoint.read8();

  // A source is always a device ID, and the clock sync table is indexed by it.
  // A node with no ID is 255 too, so this also drops a peer's ID request
  if (source >= NUM_DEVICE_IDS || source == getDeviceId()) {
    endpoint.endRead();
    return false;
  }

  return true;
}

} // namespace rvl

#endif // RVL_PROTOCOLS_HEADER_H_
