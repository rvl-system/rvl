/*
Copyright (c) Bryan Hughes <bryan@nebri.us>

This file is part of Raver Lights Messaging.

Raver Lights Messaging is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Raver Lights Messaging is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Raver Lights Messaging.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include "./rvl/platform.h"
#include "./rvl/config.h"
#include "./rvl/protocols/protocol_utils.h"

namespace ProtocolUtils {

bool isPacketForMe(uint8_t source, uint8_t destination) {
  // Ignore our own packets
  if (source == Platform::platform->getDeviceId()) {
    return false;
  }

  // Ignore multicast packets meant for a different multicast group
  if (
    destination >= CHANNEL_OFFSET && destination < 255 &&
    Platform::platform->getChannel() != destination - CHANNEL_OFFSET
  ) {
    return false;
  }

  // Ignore unicast packets meant for a different destination
  if (destination < CHANNEL_OFFSET && destination != Platform::platform->getDeviceId()) {
    return false;
  }

  // If we got here, this packet is meant for this device
  return true;
}

}  // namespace ProtocolUtils
