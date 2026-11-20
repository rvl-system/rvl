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
#include "rvl/config.hpp"
#include "rvl/platform.hpp"
#include "rvl/scenes.hpp"
#include <algorithm>
#include <string.h>

#ifdef ESP32
#include "freertos/FreeRTOS.h"
#endif

namespace rvl {

uint32_t clockOffset = 0;
uint8_t channel = 0;
DeviceMode deviceMode = DeviceMode::Receiver;
uint8_t brightness = 0;
uint16_t pixelCount = 0;
ColorOrder colorOrder = ColorOrder::RGB;
uint8_t statusLedBrightness = 0;
char nickname[NICKNAME_LENGTH + 1] = "";
uint16_t currentLimit = 0;
uint32_t identifyUntil = 0;

// State inputs used to compute current state
bool linkUp = false;
uint8_t deviceId = UNASSIGNED_DEVICE_ID;
bool clockSynced = false;
bool clockEverSynced = false;
bool controllerActive = false;
bool controllerHeard = false;
uint8_t capabilities = 0;
uint16_t maxPixelCount = 0;

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

uint32_t getAnimationFrame() {
  return getAnimationClock() / FRAME_PERIOD;
}

// Frame numbers wrap at 2^32 / FRAME_PERIOD, so the difference is scaled back
// up to 32 bits before the signed cast, or a frame just past the wrap would
// read as 49 days old
static_assert((1ULL << 32) % FRAME_PERIOD == 0);
int32_t subtractFrames(uint32_t a, uint32_t b) {
  return static_cast<int32_t>((a - b) * FRAME_PERIOD) / FRAME_PERIOD;
}

uint8_t getDeviceId() {
  return deviceId;
}

// The offset is modular, not a magnitude, so it's unsigned: corrections that
// accumulate past INT32_MAX wrap instead of overflowing. Adjusted directly
// rather than recomputed from localClock(), which would lose any millisecond
// that ticks between the two reads, always in the same direction.
//
// Below a frame this node is converging on the fleet, whose frames its scene
// bookkeeping is already in
void adjustAnimationClock(int32_t delta) {
  // The render reads the frame and the fade window together on the other core,
  // so the step and the re-base it causes must land together too
  lockState();
  uint32_t local = Platform::system->localClock();
  uint32_t before = toAnimationClock(local) / FRAME_PERIOD;
  clockOffset += delta;
  bool scheduled = false;
  if (delta >= FRAME_PERIOD || delta <= -FRAME_PERIOD) {
    uint32_t after = toAnimationClock(local) / FRAME_PERIOD;
    scheduled = Scenes::onClockStep(subtractFrames(after, before), after);
  }
  freeState();
  if (scheduled) {
    emit(EVENT_ANIMATION_UPDATED);
  }
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

uint8_t getBrightness() {
  return brightness;
}

void setBrightness(uint8_t newBrightness) {
  if (brightness != newBrightness) {
    brightness = newBrightness;
    emit(EVENT_BRIGHTNESS_UPDATED);
  }
}

uint16_t getPixelCount() {
  return pixelCount;
}

void setPixelCount(uint16_t newPixelCount) {
  newPixelCount = std::min(newPixelCount, maxPixelCount);
  if (pixelCount != newPixelCount) {
    pixelCount = newPixelCount;
    emit(EVENT_PIXEL_COUNT_UPDATED);
  }
}

ColorOrder getColorOrder() {
  return colorOrder;
}

void setColorOrder(ColorOrder newColorOrder) {
  if (colorOrder != newColorOrder) {
    colorOrder = newColorOrder;
    emit(EVENT_COLOR_ORDER_UPDATED);
  }
}

uint8_t getStatusLedBrightness() {
  return statusLedBrightness;
}

void setStatusLedBrightness(uint8_t newStatusLedBrightness) {
  if (statusLedBrightness != newStatusLedBrightness) {
    statusLedBrightness = newStatusLedBrightness;
    emit(EVENT_STATUS_LED_BRIGHTNESS_UPDATED);
  }
}

const char* getNickname() {
  return nickname;
}

void setNickname(const char* newNickname) {
  if (strncmp(nickname, newNickname, NICKNAME_LENGTH) != 0) {
    strncpy(nickname, newNickname, NICKNAME_LENGTH);
    nickname[NICKNAME_LENGTH] = '\0';
    emit(EVENT_NICKNAME_UPDATED);
  }
}

uint16_t getCurrentLimit() {
  return currentLimit;
}

void setCurrentLimit(uint16_t newCurrentLimit) {
  if (currentLimit != newCurrentLimit) {
    currentLimit = newCurrentLimit;
    emit(EVENT_CURRENT_LIMIT_UPDATED);
  }
}

void beginIdentifying() {
  identifyUntil = Platform::system->localClock() + IDENTIFY_DURATION;
}

// Pinned to now once over, so an end time left behind can't read as the future
// again once uptime passes it by 2^31 ms
void expireIdentifying() {
  uint32_t now = Platform::system->localClock();
  if (static_cast<int32_t>(now - identifyUntil) >= 0) {
    identifyUntil = now;
  }
}

bool isIdentifying() {
  return static_cast<int32_t>(identifyUntil - Platform::system->localClock()) >
      0;
}

uint32_t getIdentifyElapsedTime() {
  return IDENTIFY_DURATION - (identifyUntil - Platform::system->localClock());
}

void setCapabilities(uint8_t newCapabilities) {
  capabilities = newCapabilities;
}

uint8_t getCapabilities() {
  return capabilities;
}

void setMaxPixelCount(uint16_t newMaxPixelCount) {
  maxPixelCount = newMaxPixelCount;
}

uint16_t getMaxPixelCount() {
  return maxPixelCount;
}

bool getLinkUpState() {
  return linkUp;
}

void setLinkUpState(bool newLinkUp) {
  if (linkUp != newLinkUp) {
    linkUp = newLinkUp;
    emit(EVENT_CONNECTION_STATE_CHANGED);
  }
}

void setDeviceId(uint8_t newDeviceId) {
  deviceId = newDeviceId;
}

bool getClockSyncedState() {
  return clockSynced;
}

void setClockSyncedState(bool newClockSynced) {
  clockSynced = newClockSynced;
}

bool getClockEverSyncedState() {
  return clockEverSynced;
}

void setClockEverSyncedState(bool newClockEverSynced) {
  clockEverSynced = newClockEverSynced;
}

void setControllerActiveState(bool newControllerActive) {
  controllerActive = newControllerActive;
}

void setControllerHeardState(bool newControllerHeard) {
  controllerHeard = newControllerHeard;
}

bool isConnected() {
  return linkUp && deviceId != UNASSIGNED_DEVICE_ID;
}

RenderState getRenderState() {
  // A controller sources its own animation on its own clock, so it never waits
  if (getDeviceMode() == DeviceMode::Controller) {
    return RenderState::Current;
  }
  if (isConnected() && clockSynced && controllerActive) {
    return RenderState::Current;
  }
  if (clockEverSynced && controllerHeard) {
    return RenderState::Stale;
  }
  return RenderState::Unknown;
}

bool isLinkUp() {
  return Platform::system->isLinkUp();
}

} // namespace rvl
