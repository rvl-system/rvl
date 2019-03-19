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
#include "./RVLMessaging.h"
#include "./rvl_messaging/platform.h"
#include "./rvl_messaging/protocols/clock_sync/clock_sync.h"
#include "./rvl_messaging/protocols/giggle_pixel/giggle_pixel.h"

void RVLMessagingInit(
  RVLPlatformInterface* newPlatform,
  RVLTransportInterface* newTransport,
  RVLLogging* newLogging
) {
  Platform::init(newPlatform, newTransport, newLogging);
  ClockSync::init();
  GigglePixel::init();
}

void RVLMessagingLoop() {
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

void RVLPlatformInterface::onWaveSettingsUpdated() {
  GigglePixel::sync();
}

uint32_t RVLPlatformInterface::getClockOffset() {
  return this->clockOffset;
}
void RVLPlatformInterface::setClockOffset(uint32_t newOffset) {
  this->clockOffset = newOffset;
}

RVLDeviceMode RVLPlatformInterface::getDeviceMode() {
  return this->deviceMode;
}

void RVLPlatformInterface::setDeviceMode(RVLDeviceMode newDeviceMode) {
  this->deviceMode = newDeviceMode;
}

RVLWaveSettings* RVLPlatformInterface::getWaveSettings() {
  return &(this->waveSettings);
}

void RVLPlatformInterface::setWaveSettings(RVLWaveSettings* newWaveSettings) {
  memcpy(&(this->waveSettings), newWaveSettings, sizeof(RVLWaveSettings));
  this->onWaveSettingsUpdated();
}
