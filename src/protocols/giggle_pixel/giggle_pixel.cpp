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
#include "./protocols/giggle_pixel/giggle_pixel.h"
#include "./protocols/giggle_pixel/palette.h"
#include "./protocols/giggle_pixel/wave.h"
#include "./RaverLightsMessaging.h"
#include "./platform.h"

namespace GigglePixel {

const uint8_t protocolVersion = 1;

void init() {
  Wave::init();
  Palette::init();
}

void loop() {
  Wave::loop();
}

void sync() {
  // TODO(nebrius): Probably need to determine device Mode as well as Wave vs Palette before calling this
  Wave::sync();
}

void setClientId(uint16_t id);
void broadcastHeader(uint8_t packetType, uint8_t priority, uint16_t length);

void parsePacket() {
  Platform::debug("Parsing GigglePixel packet");
  uint8_t protocolVersion = Platform::transport->read8();
  if (protocolVersion != protocolVersion) {
    Platform::error("Received unsupported GigglePixel protocol version packet");
    return;
  }
  Platform::transport->read16();  // length
  uint8_t packetType = Platform::transport->read8();
  Platform::transport->read8();  // priority
  Platform::transport->read8();  // Reserved
  Platform::transport->read16();  // sourceId

  // Ignore our own broadcast packets
  if (Platform::platform->getDeviceMode() == RVDeviceMode::Controller) {
    return;
  }

  switch (packetType) {
    case GigglePixelPacketTypes::Wave:
      Wave::parsePacket();
      break;
    default:
      Platform::error("Unsupported packet type received");
  }
}

void broadcastHeader(uint8_t packetType, uint8_t priority, uint16_t length) {
  uint8_t* signature = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>("GLPX"));
  Platform::transport->write(signature, sizeof(uint8_t) * 4);
  Platform::transport->write8(protocolVersion);
  Platform::transport->write16(length);
  Platform::transport->write8(packetType);
  Platform::transport->write8(priority);
  Platform::transport->write8(0);  // reserved
  Platform::transport->write16(Platform::platform->getDeviceId());
}

}  // namespace GigglePixel
