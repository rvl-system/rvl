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
#include "./rvl/protocols/clock_sync/clock_sync.hpp"
#include "./rvl/protocols/system/system.hpp"
#include "./rvl/protocols/wave/wave.hpp"
#include <stdint.h>
#include <string.h>

namespace rvl {

namespace ProtocolAnimation {

void init() {
  ProtocolSystem::init();
  ProtocolClockSync::init();
  ProtocolWave::init();
}

void loop() {
  ProtocolSystem::loop();
  ProtocolWave::loop();
}

uint8_t getMulticastAddress() {
  return CHANNEL_OFFSET + getChannel();
}

void parsePacket() {
  auto& animation = Platform::system->animation();

  // The self and channel filters below both presuppose an identity
  if (!isConnected()) {
    animation.endRead();
    return;
  }

  uint8_t signature[4];
  animation.read(signature, 4);
  if (memcmp(signature, rvlaSignature, 4) != 0) {
    animation.endRead();
    return;
  }

  uint8_t version = animation.read8();
  if (version != PROTOCOL_VERSION) {
    error("Received unsupported RVLA protocol version %d, ignoring", version);
    animation.endRead();
    return;
  }

  uint8_t destination = animation.read8();
  uint8_t source = animation.read8();
  uint8_t packetType = animation.read8();
  uint8_t channel = animation.read8();
  animation.read8(); // reserved

  // A source is always a device ID, and the clock sync table is indexed by it
  if (source >= NUM_DEVICE_IDS) {
    animation.endRead();
    return;
  }

  // Ignore our own packets
  if (source == getDeviceId()) {
    animation.endRead();
    return;
  }

  // Ignore multicast packets meant for a different multicast group
  if (destination >= CHANNEL_OFFSET && destination < 255 &&
      getChannel() != channel)
  {
    animation.endRead();
    return;
  }

  // Ignore unicast packets meant for a different destination
  if (destination < CHANNEL_OFFSET && destination != getDeviceId()) {
    animation.endRead();
    return;
  }

  switch (packetType) {
  case PACKET_TYPE_SYSTEM:
    ProtocolSystem::parsePacket(source);
    break;
  case PACKET_TYPE_CLOCK_SYNC:
    ProtocolClockSync::parsePacket(source);
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

void writeHeader(uint8_t packetType, uint8_t destination) {
  auto& animation = Platform::system->animation();
  animation.beginChannelWrite();
  animation.write(rvlaSignature, 4);
  animation.write8(PROTOCOL_VERSION);
  animation.write8(destination);
  animation.write8(getDeviceId());
  animation.write8(packetType);
  animation.write8(getChannel());
  animation.write8(0);
}

// On the wire a broadcast is a channel write with destination 255, which the
// receiving side exempts from the channel filter
void beginBroadcastWrite(uint8_t packetType) {
  writeHeader(packetType, 255);
}

void beginChannelWrite(uint8_t packetType) {
  writeHeader(packetType, getMulticastAddress());
}

} // namespace ProtocolAnimation

} // namespace rvl
