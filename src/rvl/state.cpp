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

#include "./rvl/state.hpp"
#include "rvl/platform.hpp"
#include <string.h>

#ifdef ESP32
#include "freertos/FreeRTOS.h"
#endif

namespace rvl {

int32_t clockOffset = 0;
uint8_t channel = 0;
DeviceMode deviceMode = DeviceMode::Receiver;
RVLWaveSettings waveSettings;
bool powerState = false;
uint8_t brightness = 0;
bool remoteBrightness = false;
bool synchronized = false;

#ifdef ESP32
portMUX_TYPE stateMux = portMUX_INITIALIZER_UNLOCKED;
#endif

// Single-threaded platforms have no other task to exclude, so these are no-ops
// there
void lockState() {
#ifdef ESP32
  portENTER_CRITICAL(&stateMux);
#endif
}

void freeState() {
#ifdef ESP32
  portEXIT_CRITICAL(&stateMux);
#endif
}

// Computed live rather than cached per loop tick: the render loop runs on a
// different task than the network loop, so a cached value would be stale by an
// arbitrary, drifting fraction of a tick on each node
uint32_t getAnimationClock() {
  return toAnimationClock(Platform::system->localClock());
}

uint32_t toAnimationClock(uint32_t localTime) {
  return localTime + clockOffset;
}

uint8_t getDeviceId() {
  return Platform::system->getDeviceId();
}

void setAnimationClock(uint32_t newClock) {
  clockOffset = newClock - Platform::system->localClock();
}

uint8_t getChannel() {
  return channel;
}

void setChannel(uint8_t newChannel) {
  if (channel != newChannel) {
    channel = newChannel;
    emit(EVENT_CHANNEL_UPDATED);
  }
}

DeviceMode getDeviceMode() {
  return deviceMode;
}

void setDeviceMode(DeviceMode newDeviceMode) {
  if (deviceMode != newDeviceMode) {
    deviceMode = newDeviceMode;
    emit(EVENT_DEVICE_MODE_UPDATED);
  }
}

RVLWaveSettings* getWaveSettings() {
  return &waveSettings;
}

void setWaveSettings(RVLWaveSettings* newWaveSettings) {
  lockState();
  memcpy(&waveSettings, newWaveSettings, sizeof(RVLWaveSettings));
  freeState();
  emit(EVENT_WAVE_SETTINGS_UPDATED);
}

bool getPowerState() {
  return powerState;
}

void setPowerState(bool newPowerState) {
  if (powerState != newPowerState) {
    powerState = newPowerState;
    emit(EVENT_POWER_STATE_UPDATED);
  }
}

uint8_t getBrightness() {
  return brightness;
}

void setBrightness(uint8_t newBrightness) {
  if (brightness != newBrightness) {
    brightness = newBrightness;
    emit(EVENT_BRIGHTNESS_UPDATED);
  }
}

bool getRemoteBrightnessState() {
  return remoteBrightness;
}

void setRemoteBrightnessState(bool newRemoteBrightness) {
  if (remoteBrightness != newRemoteBrightness) {
    remoteBrightness = newRemoteBrightness;
    emit(EVENT_REMOTE_BRIGHTNESS_UPDATED);
  }
}

bool getSynchronizationState() {
  if (getDeviceMode() == DeviceMode::Controller) {
    return true;
  }
  return synchronized;
}

void setSynchronizationState(bool newSynchronized) {
  synchronized = newSynchronized;
}

bool isNetworkConnected() {
  return Platform::system->isConnected();
}

} // namespace rvl
