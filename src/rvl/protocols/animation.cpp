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

#include "./rvl/protocols/animation.hpp"
#include "./rvl.hpp"
#include "./rvl/config.hpp"
#include "./rvl/platform.hpp"
#include "./rvl/protocols/header.hpp"
#include "./rvl/protocols/network_state.hpp"
#include "./rvl/protocols/wave/wave.hpp"
#include <stdint.h>

namespace rvl {

namespace ProtocolAnimation {

uint32_t nextSyncTime;

// One sender for every type, so two types can never race on the same channel
void sync() {
  if (getDeviceMode() != DeviceMode::Controller || !isConnected()) {
    return;
  }
  switch (getAnimationType()) {
  case AnimationType::Off:
    beginChannelWrite(PACKET_TYPE_OFF);
    break;
  case AnimationType::Wave:
    beginChannelWrite(PACKET_TYPE_WAVE_ANIMATION);
    ProtocolWave::write();
    break;
  }
  Platform::system->animation().endWrite();
}

void init() {
  nextSyncTime = Platform::system->localClock();
  on(EVENT_ANIMATION_UPDATED, sync);
}

// The timer advances whether or not this node sends, so it never falls 2^31 ms
// behind the clock and reads as the future
void loop() {
  uint32_t now = Platform::system->localClock();
  if (static_cast<int32_t>(now - nextSyncTime) < 0) {
    return;
  }
  nextSyncTime = now + CLIENT_SYNC_INTERVAL;
  sync();
}

void parsePacket() {
  auto& animation = Platform::system->animation();

  // The self filter presupposes an identity
  if (!isConnected()) {
    animation.endRead();
    return;
  }

  uint8_t source;
  uint8_t packetType;
  if (!readHeader(animation, rvlaSignature, source, packetType)) {
    return;
  }
  uint8_t channel = animation.read8();
  animation.read8(); // reserved

  if (channel != getChannel()) {
    animation.endRead();
    return;
  }

  switch (packetType) {
  case PACKET_TYPE_OFF:
    if (NetworkState::isControllerNode(source)) {
      setOff();
    }
    break;
  case PACKET_TYPE_WAVE_ANIMATION:
    ProtocolWave::parsePacket(source);
    break;
  default:
    error("Received unknown RVLA packet type %d", packetType);
    break;
  }
  animation.endRead();
}

void beginChannelWrite(uint8_t packetType) {
  auto& animation = Platform::system->animation();
  animation.beginChannelWrite();
  animation.write(rvlaSignature, 4);
  animation.write8(PROTOCOL_VERSION);
  animation.write8(getDeviceId());
  animation.write8(packetType);
  animation.write8(getChannel());
  animation.write8(0);
}

} // namespace ProtocolAnimation

} // namespace rvl
