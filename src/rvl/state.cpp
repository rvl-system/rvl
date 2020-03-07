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

#include "./rvl/state.h"
#include "./rvl/platform.h"

namespace rvl {

uint32_t animationClock;
int32_t clockOffset = 0;
uint8_t channel = 0;
RVLDeviceMode deviceMode = RVLDeviceMode::Receiver;
RVLWaveSettings waveSettings;
bool powerState = 0;
uint8_t brightness = 0;
bool synchronized = 0;

void stateLoop() {
  animationClock = millis() + clockOffset;
}

uint32_t getAnimationClock() {
  return animationClock;
}

void setAnimationClock(uint32_t newClock) {
  clockOffset = newClock - millis();
  emit(EVENT_ANIMATION_CLOCK_UPDATED);
}

uint8_t getChannel() {
  return channel;
}

void setChannel(uint8_t newChannel) {
  channel = newChannel;
  emit(EVENT_CHANNEL_UPDATED);
}

RVLDeviceMode getDeviceMode() {
  return deviceMode;
}

void setDeviceMode(RVLDeviceMode newDeviceMode) {
  deviceMode = newDeviceMode;
  emit(EVENT_DEVICE_MODE_UPDATED);
}

RVLWaveSettings* getWaveSettings() {
  return &waveSettings;
}

void setWaveSettings(RVLWaveSettings* newWaveSettings) {
  memcpy(&waveSettings, newWaveSettings, sizeof(RVLWaveSettings));
  emit(EVENT_WAVE_SETTINGS_UPDATED);
}

bool getPowerState() {
  return powerState;
}

void setPowerState(bool newPowerState) {
  powerState = newPowerState;
  emit(EVENT_POWER_STATE_UPDATED);
}

uint8_t getBrightness() {
  return brightness;
}

void setBrightness(uint8_t newBrightness) {
  brightness = newBrightness;
  emit(EVENT_BRIGHTNESS_UPDATED);
}

bool getSynchronizationState() {
  return synchronized;
}

void setSynchronizationState(bool newSynchronized) {
  synchronized = newSynchronized;
  emit(EVENT_SYNCHRONIZATION_STATE_UPDATED);
}

bool isNetworkConnected() {
  return Platform::transport->isConnected();
}

}  // namespace rvl
