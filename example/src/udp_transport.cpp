/*
Copyright (c) Bryan Hughes <bryan@nebri.us>

This file is part of Raver Lights Messaging.

Raver Lights Messaging is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Raver Lights Messaging is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Raver Lights Messaging.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "./udp_transport.h"

WiFiUDP* udpTransport;
IPAddress SERVER_IP(192, 168, 1, 1);
IPAddress GATEWAY(192, 168, 1, 255);
IPAddress SUBNET(255, 255, 255, 0);

UDPTransport::UDPTransport(WiFiUDP* newUDP) {
  udpTransport = newUDP;
}

void UDPTransport::beginWrite() {
  udpTransport->beginPacket(GATEWAY, SERVER_PORT);
}

void UDPTransport::write8(uint8_t data) {
  udpTransport->write(data);
}

void UDPTransport::write16(uint16_t data) {
  udpTransport->write(data >> 8);
  udpTransport->write(data & 0xFF);
}

void UDPTransport::write32(uint32_t data) {
  udpTransport->write(data >> 24);
  udpTransport->write(data >> 16 & 0xFF);
  udpTransport->write(data >> 8 & 0xFF);
  udpTransport->write(data & 0xFF);
}

void UDPTransport::write(uint8_t* data, uint16_t length) {
  udpTransport->write(data, length);
}

void UDPTransport::endWrite() {
  udpTransport->endPacket();
}

uint16_t UDPTransport::parsePacket() {
  return udpTransport->parsePacket();
}

uint8_t UDPTransport::read8() {
  return udpTransport->read();
}

uint16_t UDPTransport::read16() {
  uint16_t val = 0;
  val |= udpTransport->read() << 8;
  val |= udpTransport->read();
  return val;
}

uint32_t UDPTransport::read32() {
  uint32_t val = 0;
  val |= udpTransport->read() << 24;
  val |= udpTransport->read() << 16;
  val |= udpTransport->read() << 8;
  val |= udpTransport->read();
  return val;
}

void UDPTransport::read(uint8_t* buffer, uint16_t length) {
  udpTransport->read(buffer, length);
}