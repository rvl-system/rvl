/*
Copyright (c) Bryan Hughes <bryan@nebri.us>

This file is part of RVL Arduino.

RVL Arduino is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RVL Arduino is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RVL Arduino.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <list>
#include "./rvl.h"
#include "./rvl/config.h"
#include "./rvl/platform.h"
#include "./rvl/protocols/protocol.h"
#include "./rvl/protocols/wave/wave.h"
#include "./rvl/protocols/system/system.h"

uint32_t animationClock;
int32_t clockOffset = 0;

RVLPlatformInterface* rvlPlatform;

void RVLMessagingInit(
  RVLPlatformInterface* newPlatform,
  RVLTransportInterface* newTransport
) {
  rvlPlatform = newPlatform;
  Platform::init(newPlatform, newTransport);
  Protocol::init();
}

void RVLMessagingLoop() {
  animationClock = millis() + clockOffset;
  if (!Platform::transport->isNetworkAvailable()) {
    return;
  }

  int packetSize = Platform::transport->parsePacket();
  if (packetSize != 0) {
    uint8_t receivedSignature[4];
    Platform::transport->read(receivedSignature, 4);
    if (memcmp(receivedSignature, signature, 4) == 0) {
      Protocol::parsePacket();
    }
  }

  Protocol::loop();
}

void RVLPlatformInterface::onWaveSettingsUpdated() {
  ProtocolWave::sync();
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
  ProtocolSystem::sync();
}

void RVLPlatformInterface::onBrightnessUpdated() {
  ProtocolSystem::sync();
}

void RVLPlatformInterface::onSynchronizationStateUpdated() {
  // Do nothing
}

uint32_t RVLPlatformInterface::getAnimationClock() {
  return animationClock;
}

void RVLPlatformInterface::setAnimationClock(uint32_t newClock) {
  clockOffset = newClock - millis();
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

bool RVLPlatformInterface::getSynchronizationState() {
  return this->synchronized;
}

void RVLPlatformInterface::setSynchronizationState(bool synchronized) {
  this->synchronized = synchronized;
  this->onSynchronizationStateUpdated();
}
