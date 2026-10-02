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
#include "./rvl/protocols/parametric/parametric.hpp"
#include <algorithm>
#include <optional>
#include <stdint.h>
#include <variant>

namespace rvl {

namespace ProtocolAnimation {

const uint16_t HEADER_LENGTH = 9;
const uint16_t SCENE_PREFIX_LENGTH = 6;
const uint16_t OFF_PACKET_LENGTH = HEADER_LENGTH + SCENE_PREFIX_LENGTH;
const uint16_t PARAMETRIC_PACKET_LENGTH =
    OFF_PACKET_LENGTH + sizeof(RVLParametricSettings);

uint32_t nextSyncTime;

// 0 for a type this node doesn't know
uint16_t packetLength(uint8_t packetType) {
  switch (packetType) {
  case PACKET_TYPE_OFF:
    return OFF_PACKET_LENGTH;
  case PACKET_TYPE_PARAMETRIC_ANIMATION:
    return PARAMETRIC_PACKET_LENGTH;
  default:
    return 0;
  }
}

void writeScene(const RVLScene& scene) {
  auto& animation = Platform::system->animation();
  const auto* settings = std::get_if<RVLParametricSettings>(&scene.content);
  beginChannelWrite(
      settings != nullptr ? PACKET_TYPE_PARAMETRIC_ANIMATION : PACKET_TYPE_OFF);
  animation.write32(scene.start);
  animation.write8(0); // reserved
  animation.write8(scene.fade);
  if (settings != nullptr) {
    ProtocolParametric::write(*settings);
  }
  animation.endWrite();
}

// Current, then pending, one packet each. Returns whether anything was sent
bool sync() {
  if (getDeviceMode() != DeviceMode::Controller || !isConnected() ||
      !getClockEverSyncedState())
  {
    return false;
  }
  bool sent = false;
  RVLScene current = getCurrentScene();
  // The boot scene, the only one with no fade, is this node's own and never
  // the fleet's
  if (current.fade != 0) {
    writeScene(current);
    sent = true;
  }
  if (std::optional<RVLScene> pending = getPendingScene()) {
    writeScene(*pending);
    sent = true;
  }
  return sent;
}

// A newly scheduled scene goes out at once and again REPEAT_SEND_FRAMES later,
// on the frame boundary, by pulling the periodic timer forward
void onAnimationUpdated() {
  if (!sync()) {
    return;
  }
  // One local read, so the two clocks can't straddle a millisecond tick
  uint32_t now = Platform::system->localClock();
  nextSyncTime = now + REPEAT_SEND_FRAMES * FRAME_PERIOD -
      toAnimationClock(now) % FRAME_PERIOD;
}

void init() {
  nextSyncTime = Platform::system->localClock();
  on(EVENT_ANIMATION_UPDATED, onAnimationUpdated);
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

void parsePacket(uint16_t length) {
  auto& animation = Platform::system->animation();

  // The self filter presupposes an identity
  if (!isConnected()) {
    animation.endRead();
    return;
  }

  uint8_t source;
  uint8_t packetType;
  if (!readHeader(animation, rvlaSignature, RVLA_VERSION, source, packetType)) {
    return;
  }
  uint8_t channel = animation.read8();
  animation.read8(); // reserved

  uint16_t expectedLength = packetLength(packetType);
  if (expectedLength == 0) {
    error("Received unknown RVLA packet type %d", packetType);
    animation.endRead();
    return;
  }
  // Every sender is this fleet's own, so a wrong length is a bug. Checked
  // before the payload is read, which would run past the end of a short packet
  if (length != expectedLength) {
    error("Received RVLA packet type %d of %d bytes, expected %d, ignoring",
        packetType, length, expectedLength);
    animation.endRead();
    return;
  }

  if (channel != getChannel()) {
    animation.endRead();
    return;
  }
  // A start means nothing before the first sync, and the re-send delivers
  // within an interval of it. Before the controller check, so a dropped packet
  // adopts no controller
  if (!getClockEverSyncedState() || !NetworkState::isControllerNode(source)) {
    animation.endRead();
    return;
  }

  uint32_t start = animation.read32();
  animation.read8(); // reserved
  uint8_t fade = std::max<uint8_t>(animation.read8(), MIN_FADE_FRAMES);
  RVLSceneContent content = RVLOff{};
  if (packetType == PACKET_TYPE_PARAMETRIC_ANIMATION) {
    RVLParametricSettings settings;
    if (!ProtocolParametric::read(settings)) {
      animation.endRead();
      return;
    }
    content = settings;
  }
  animation.endRead();
  scheduleScene({start, fade, content});
}

void beginChannelWrite(uint8_t packetType) {
  auto& animation = Platform::system->animation();
  animation.beginChannelWrite();
  animation.write(rvlaSignature, 4);
  animation.write8(RVLA_VERSION);
  animation.write8(getDeviceId());
  animation.write8(packetType);
  animation.write8(getChannel());
  animation.write8(0);
}

} // namespace ProtocolAnimation

} // namespace rvl
