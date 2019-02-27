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
#include "./RaverLightsMessaging.h"
#include "./platform.h"
#include "./protocols/clock_sync/clock_sync.h"
#include "./protocols/giggle_pixel/giggle_pixel.h"

void init(RVPlatformInterface* newPlatform, RVTransportInterface* newTransport, RVLoggingInterface* newLogging) {
  Platform::init(newPlatform, newTransport, newLogging);
  ClockSync::init();
  GigglePixel::init();
}

void loop() {
  int packetSize = Platform::transport->parsePacket();
  if (packetSize == 0) {
    return;
  }
  uint8_t signature[4];
  Platform::transport->read(signature, 4);
  if (
    signature[0] == ClockSync::signature[0] &&
    signature[1] == ClockSync::signature[1] &&
    signature[2] == ClockSync::signature[2] &&
    signature[3] == ClockSync::signature[3]
  ) {
    ClockSync::parsePacket();
  } else if (
    signature[0] == GigglePixel::signature[0] &&
    signature[1] == GigglePixel::signature[1] &&
    signature[2] == GigglePixel::signature[2] &&
    signature[3] == GigglePixel::signature[3]
  ) {
    GigglePixel::parsePacket();
  }

  ClockSync::loop();
  GigglePixel::loop();
}
