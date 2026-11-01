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

#ifndef RVL_STATE_H_
#define RVL_STATE_H_

#include "./rvl.hpp"
#include <stdint.h>

namespace rvl {

enum class DeviceMode : uint8_t { Controller, Receiver };

// Unknown: nothing to render yet. Current: rendering what the fleet is showing.
// Stale: what was current has lapsed, so keep rendering it
enum class RenderState : uint8_t { Unknown, Current, Stale };

// The order a strip reads its three bytes in. The values are the wire's
enum class ColorOrder : uint8_t { RGB, RBG, GRB, GBR, BRG, BGR };

// The properties a node can be configured with over the wire, numbered as the
// wire numbers them
enum class Property : uint8_t {
  Channel,
  Brightness,
  PixelCount,
  ColorOrder,
  StatusLedBrightness,
  Nickname
};
#define NUM_PROPERTIES 6

// The property's bit in a capabilities byte
constexpr uint8_t capabilityBit(Property property) {
  return 1 << static_cast<uint8_t>(property);
}

uint32_t getAnimationClock();
void adjustAnimationClock(int32_t delta);
uint32_t toAnimationClock(uint32_t localTime);
uint32_t getAnimationFrame();
int32_t subtractFrames(uint32_t a, uint32_t b);

uint8_t getDeviceId();

uint8_t getChannel();
void setChannel(uint8_t channel);

DeviceMode getDeviceMode();
void setDeviceMode(DeviceMode newDeviceMode);

void lockState();
void freeState();

uint8_t getBrightness();
void setBrightness(uint8_t newBrightness);

// Clamped to the maximum the app set
uint16_t getPixelCount();
void setPixelCount(uint16_t newPixelCount);

ColorOrder getColorOrder();
void setColorOrder(ColorOrder newColorOrder);

uint8_t getStatusLedBrightness();
void setStatusLedBrightness(uint8_t newStatusLedBrightness);

// At most NICKNAME_LENGTH characters, always terminated
const char* getNickname();
void setNickname(const char* newNickname);

// Set by an Identify, for IDENTIFY_DURATION of the local clock. The render
// loops read it from the other task, so only the background task writes the
// end time: expireIdentifying() pins it to now once over, every loop
void beginIdentifying();
void expireIdentifying();
bool isIdentifying();

// State inputs
// The properties this node honors over the wire, as capability bits, and the
// most pixels its buffers hold, both set by the app before anything else
void setCapabilities(uint8_t newCapabilities);
uint8_t getCapabilities();
void setMaxPixelCount(uint16_t newMaxPixelCount);
uint16_t getMaxPixelCount();
bool getLinkUpState();
void setLinkUpState(bool linkUp);
void setDeviceId(uint8_t deviceId);
bool getClockSyncedState();
void setClockSyncedState(bool clockSynced);
bool getClockEverSyncedState();
void setClockEverSyncedState(bool clockEverSynced);
void setControllerActiveState(bool controllerActive);
void setControllerHeardState(bool controllerHeard);

// Indicates whether or not we can communicate with other nodes
bool isConnected();

RenderState getRenderState();

// Indicates whether or not the transport mechanism is available, e.g. whether
// or not we're connected to the WiFi AP
bool isLinkUp();

} // namespace rvl

#endif // RVL_STATE_H_
