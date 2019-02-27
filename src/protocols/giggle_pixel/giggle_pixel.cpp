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

namespace GigglePixel {

const uint8_t protocolVersion = 1;

RaverLightsMessaging::TransportInterface* transport;

void init(RaverLightsMessaging::TransportInterface* newTransport) {
  transport = newTransport;
  Wave::init(newTransport);
  Palette::init(newTransport);
}

void loop() {
  Wave::loop();
}

void setClientId(uint16_t id);
void broadcastHeader(uint8_t packetType, uint8_t priority, uint16_t length);

void parsePacket() {
  Logging::debug("Parsing GigglePixel packet");
  uint8_t protocolVersion = transport->read8();
  if (protocolVersion != protocolVersion) {
    Logging::error("Received unsupported GigglePixel protocol version packet");
    return;
  }
  transport->read16();  // length
  uint8_t packetType = transport->read8();
  transport->read8();  // priority
  transport->read8();  // Reserved
  transport->read16();  // sourceId

  // Ignore our own broadcast packets
  if (State::getSettings()->mode == Codes::Mode::Controller) {
    return;
  }

  switch (packetType) {
    case Codes::GigglePixelPacketTypes::Wave:
      Wave::parsePacket();
      break;
    default:
      Logging::error("Unsupported packet type received: %d", packetType);
  }
}

void broadcastHeader(uint8_t packetType, uint8_t priority, uint16_t length) {
  uint8_t* signature = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>("GLPX"));
  transport->write(signature, sizeof(uint8_t) * 4);
  transport->write8(protocolVersion);
  transport->write16(length);
  transport->write8(packetType);
  transport->write8(priority);
  transport->write8(0);  // reserved
  transport->write16(State::getSettings()->id);
}

}  // namespace GigglePixel
