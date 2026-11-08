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

#include "./rvl/protocols/configuration/configuration.hpp"
#include "./rvl.hpp"
#include "./rvl/config.hpp"
#include "./rvl/platform.hpp"
#include "./rvl/protocols/infrastructure.hpp"
#include <stdint.h>
#include <string.h>

namespace rvl {

namespace ProtocolConfiguration {

const uint16_t QUERY_PACKET_LENGTH = ProtocolInfrastructure::HEADER_LENGTH;
const uint16_t IDENTIFY_PACKET_LENGTH = ProtocolInfrastructure::HEADER_LENGTH;
// The property byte, then the property's value
const uint16_t CONFIGURE_PREFIX_LENGTH =
    ProtocolInfrastructure::HEADER_LENGTH + 1;
const uint16_t PROPERTY_VALUE_LENGTHS[NUM_PROPERTIES] = {
    1, 1, 2, 1, 1, NICKNAME_LENGTH, 2};

void sendStatus(uint8_t destination) {
  auto& infrastructure = Platform::system->infrastructure();
  ProtocolInfrastructure::beginBroadcastWrite(
      RVLI_PACKET_TYPE_STATUS, destination);
  infrastructure.write8(getCapabilities());
  infrastructure.write8(getChannel());
  infrastructure.write8(getBrightness());
  infrastructure.write16(getPixelCount());
  infrastructure.write8(static_cast<uint8_t>(getColorOrder()));
  infrastructure.write8(getStatusLedBrightness());
  uint8_t nickname[NICKNAME_LENGTH] = {0};
  strncpy(reinterpret_cast<char*>(nickname), getNickname(), NICKNAME_LENGTH);
  infrastructure.write(nickname, NICKNAME_LENGTH);
  infrastructure.write16(getCurrentLimit());
  infrastructure.endWrite();
}

void parseQuery(uint8_t source, uint16_t length) {
  if (length != QUERY_PACKET_LENGTH) {
    error("Received query of %d bytes, expected %d, ignoring", length,
        QUERY_PACKET_LENGTH);
    return;
  }
  sendStatus(source);
}

// Returns whether the value was taken: a bad one is a sender bug, and dropped
bool applyProperty(Property property) {
  auto& infrastructure = Platform::system->infrastructure();
  switch (property) {
  case Property::Channel: {
    uint8_t channel = infrastructure.read8();
    if (channel >= NUM_CHANNELS) {
      error("Received channel %d, ignoring", channel);
      return false;
    }
    setChannel(channel);
    break;
  }
  case Property::Brightness:
    setBrightness(infrastructure.read8());
    break;
  case Property::PixelCount:
    setPixelCount(infrastructure.read16());
    break;
  case Property::ColorOrder: {
    uint8_t order = infrastructure.read8();
    if (order > static_cast<uint8_t>(ColorOrder::BGR)) {
      error("Received color order %d, ignoring", order);
      return false;
    }
    setColorOrder(static_cast<ColorOrder>(order));
    break;
  }
  case Property::StatusLedBrightness:
    setStatusLedBrightness(infrastructure.read8());
    break;
  case Property::Nickname: {
    char nickname[NICKNAME_LENGTH + 1];
    infrastructure.read(reinterpret_cast<uint8_t*>(nickname), NICKNAME_LENGTH);
    nickname[NICKNAME_LENGTH] = '\0';
    setNickname(nickname);
    break;
  }
  case Property::CurrentLimit:
    setCurrentLimit(infrastructure.read16());
    break;
  }
  return true;
}

void parseConfigure(uint8_t source, uint16_t length) {
  auto& infrastructure = Platform::system->infrastructure();
  uint8_t propertyByte = infrastructure.read8();
  if (propertyByte >= NUM_PROPERTIES) {
    error("Received configure for unknown property %d", propertyByte);
    return;
  }
  // Every sender is this fleet's own, so a wrong length is a bug. Checked
  // before the value is read, which would run past the end of a short packet
  uint16_t expectedLength =
      CONFIGURE_PREFIX_LENGTH + PROPERTY_VALUE_LENGTHS[propertyByte];
  if (length != expectedLength) {
    error("Received configure for property %d of %d bytes, expected %d, "
          "ignoring",
        propertyByte, length, expectedLength);
    return;
  }
  // A property this node doesn't honor is left alone, and the status says so
  Property property = static_cast<Property>(propertyByte);
  bool honored = (getCapabilities() & capabilityBit(property)) != 0;
  if (honored && !applyProperty(property)) {
    return;
  }
  sendStatus(source);
}

void parseIdentify(uint8_t source, uint16_t length) {
  if (length != IDENTIFY_PACKET_LENGTH) {
    error("Received identify of %d bytes, expected %d, ignoring", length,
        IDENTIFY_PACKET_LENGTH);
    return;
  }
  beginIdentifying();
  sendStatus(source);
}

} // namespace ProtocolConfiguration

} // namespace rvl
