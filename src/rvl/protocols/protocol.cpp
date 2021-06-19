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

#include "./rvl/protocols/protocol.hpp"
#include "./rvl/config.hpp"
#include "./rvl/platform.hpp"
#include "./rvl/protocols/clock_sync/clock_sync.hpp"
#include "./rvl/protocols/discover/discover.hpp"
#include "./rvl/protocols/network_state.hpp"
#include "./rvl/protocols/system/system.hpp"
#include "./rvl/protocols/wave/wave.hpp"
#include <stdint.h>

namespace rvl {

namespace Protocol {

#define PROTOCOL_VERSION 1

/*
Signature: 4 bytes = "RVLX"
Version: 1 byte = PROTOCOL_VERSION
Destination: 1 byte = 0-239: individual device, 240-254: multicast, 255:
broadcast Source: 1 byte = the address of the device that sent the message
Packet type: 1 byte = 1: System, 2: Discover, 3: Clock Sync, 4: Wave Animation
Channel: 1 byte = the channel this system is on (needed for disambiguation on
systems running multiple controller instances) Reserved: 1 bytes = Reserved for
future use
*/

void init() {
  NetworkState::init();
  ProtocolSystem::init();
  ProtocolDiscover::init();
  ProtocolClockSync::init();
  ProtocolWave::init();
}

void loop() {
  NetworkState::loop();
  ProtocolSystem::loop();
  ProtocolDiscover::loop();
  ProtocolClockSync::loop();
  ProtocolWave::loop();
}

uint8_t getMulticastAddress() {
  return CHANNEL_OFFSET + getChannel();
}

void parsePacket() {
  uint8_t version = Platform::system->read8();
  if (version != PROTOCOL_VERSION) {
    error("Received unsupported Raver Lights protocol packet version %d, "
          "ignoring",
        version);
    return;
  }

  uint8_t deviceId = Platform::system->getDeviceId();

  uint8_t destination = Platform::system->read8(); // destination
  uint8_t source = Platform::system->read8(); // source
  uint8_t packetType = Platform::system->read8();
  uint8_t channel = Platform::system->read8();
  Platform::system->read8(); // reserved

  // Ignore our own packets
  if (source == deviceId) {
    return;
  }

  // Refresh this node's presence in the network map
  NetworkState::refreshNode(source);

  // Ignore multicast packets meant for a different multicast group
  if (destination >= CHANNEL_OFFSET && destination < 255 &&
      getChannel() != channel)
  {
    return;
  }

  // Ignore unicast packets meant for a different destination
  if (destination < CHANNEL_OFFSET && destination != deviceId) {
    return;
  }

  switch (packetType) {
  case PACKET_TYPE_SYSTEM:
    ProtocolSystem::parsePacket(source);
    break;
  case PACKET_TYPE_DISCOVER:
    ProtocolDiscover::parsePacket(source);
    break;
  case PACKET_TYPE_CLOCK_SYNC:
    ProtocolClockSync::parsePacket(source);
    break;
  case PACKET_TYPE_WAVE_ANIMATION:
    ProtocolWave::parsePacket(source);
    break;
  default:
    error("Received unknown subpacket type %d", packetType);
    break;
  }
}

void sendHeader(uint8_t packetType, uint8_t destination) {
  Platform::system->write(signature, 4);
  Platform::system->write8(PROTOCOL_VERSION);
  Platform::system->write8(destination);
  Platform::system->write8(Platform::system->getDeviceId());
  Platform::system->write8(packetType);
  Platform::system->write8(getChannel());
  Platform::system->write8(0);
}

void sendBroadcastHeader(uint8_t packetType) {
  sendHeader(packetType, 255);
}

void sendMulticastHeader(uint8_t packetType) {
  sendHeader(packetType, getMulticastAddress());
}

} // namespace Protocol

} // namespace rvl
