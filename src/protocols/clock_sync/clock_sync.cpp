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
#include "./protocols/clock_sync/clock_sync.h"
#include "./RaverLightsMessaging.h"

namespace ClockSync {

const uint8_t protocolVersion = 1;

RaverLightsMessaging::TransportInterface* transport;

void init(RaverLightsMessaging::TransportInterface* newTransport) {
  transport = newTransport;
}

void loop() {
  // Not implemented yet
}

/*
Signature: 4 bytes = "CLKS"
Version: 1 byte = 1
Type: 1 byte = 1:reference, 2:response
Sequence: 2 bytes = always incrementing
Clock: 4 bytes = running clock, relative to app start
ClientID: 2 bytes = matches ClientID in GigglePixel, or 0 for transmitter
*/

bool parsePacket() {
  Logging::debug("Parsing Clock Sync packet");
  uint8_t version = transport->read8();
  if (protocolVersion != version) {
    return false;
  }
  transport->read8();  // type
  transport->read16();  // seq
  uint32_t commandTime = transport->read32();
  transport->read16();  // clientId

  State::setClockOffset(static_cast<int32_t>(commandTime) - static_cast<int32_t>(millis()));

  return true;
}

}  // namespace ClockSync
