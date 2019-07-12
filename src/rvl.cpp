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
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "./rvl.h"
#include "./rvl/platform.h"
#include "./rvl/protocols/clock_sync/clock_sync.h"
#include "./rvl/protocols/giggle_pixel/giggle_pixel.h"
#include "./rvl/protocols/rvl_system/rvl_system.h"

uint32_t animationClock;

RVLPlatformInterface* rvlPlatform;

void RVLMessagingInit(
  RVLPlatformInterface* newPlatform,
  RVLTransportInterface* newTransport,
  RVLLogging* newLogging
) {
  rvlPlatform = newPlatform;
  Platform::init(newPlatform, newTransport, newLogging);
  RVLSystem::init();
  ClockSync::init();
  GigglePixel::init();
}

bool compareSignature(const uint8_t* reference, uint8_t* received, size_t len) {
  return memcmp(reference, received, len) == 0;
}

void RVLMessagingLoop() {
  animationClock = rvlPlatform->getLocalTime() + rvlPlatform->getClockOffset();
  if (!rvlPlatform->isNetworkAvailable()) {
    return;
  }

  int packetSize = Platform::transport->parsePacket();
  if (packetSize != 0) {
    uint8_t signature[4];
    Platform::transport->read(signature, 4);
    if (compareSignature(ClockSync::signature, signature, 4)) {
      ClockSync::parsePacket();
    } else if (compareSignature(GigglePixel::signature, signature, 4)) {
      GigglePixel::parsePacket();
    } else if (compareSignature(RVLSystem::signature, signature, 4)) {
      RVLSystem::parsePacket();
    }
  }

  RVLSystem::loop();
  ClockSync::loop();
  GigglePixel::loop();
}

RVLLogging::RVLLogging(RVLLoggingInterface* iface, RVLLogLevel level) {
  this->interface = iface;
  this->logLevel = level;
}

void RVLLogging::log(const char *s) {
  this->interface->print(s);
}

void RVLLogging::log(const char *s, va_list argptr) {
  int bufferLength = strlen(s) * 3;
  char str[bufferLength];
  vsnprintf(str, bufferLength, s, argptr);
  this->interface->print(str);
}

void RVLLogging::error(const char *s, ...) {
  if (this->logLevel >= RVLLogLevel::Error) {
    this->interface->print("[error]: ");
    va_list argptr;
    va_start(argptr, s);
    this->log(s, argptr);
    this->interface->println();
  }
}

void RVLLogging::info(const char *s, ...) {
  if (this->logLevel >= RVLLogLevel::Info) {
    this->interface->print("[info ]: ");
    va_list argptr;
    va_start(argptr, s);
    this->log(s, argptr);
    this->interface->println();
  }
}

void RVLLogging::debug(const char *s, ...) {
  if (this->logLevel >= RVLLogLevel::Debug) {
    this->interface->print("[debug ]: ");
    va_list argptr;
    va_start(argptr, s);
    this->log(s, argptr);
    this->interface->println();
  }
}

void RVLPlatformInterface::onWaveSettingsUpdated() {
  GigglePixel::sync();
}

void RVLPlatformInterface::onDeviceModeUpdated() {
  // Do nothing
}

void RVLPlatformInterface::onClockOffsetUpdated() {
  // Do nothing
}

void RVLPlatformInterface::onChannelUpdated() {
  // Do nothing
}

void RVLPlatformInterface::onPowerStateUpdated() {
  RVLSystem::sync();
}

void RVLPlatformInterface::onBrightnessUpdated() {
  RVLSystem::sync();
}

uint32_t RVLPlatformInterface::getClockOffset() {
  return this->clockOffset;
}
void RVLPlatformInterface::setClockOffset(uint32_t newOffset) {
  this->clockOffset = newOffset;
}

uint32_t RVLPlatformInterface::getAnimationClock() {
  return animationClock;
}

uint8_t RVLPlatformInterface::getChannel() {
  return this->channel;
}

void RVLPlatformInterface::setChannel(uint8_t channel) {
  this->channel = channel;
  this->onChannelUpdated();
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

bool RVLPlatformInterface::getPowerState() {
  return this->powerState;
}

void RVLPlatformInterface::setPowerState(bool newPowerState) {
  this->powerState = newPowerState;
  this->onPowerStateUpdated();
}

uint8_t RVLPlatformInterface::getBrightness() {
  return this->brightness;
}

void RVLPlatformInterface::setBrightness(uint8_t newBrightness) {
  this->brightness = newBrightness;
  this->onPowerStateUpdated();
}
