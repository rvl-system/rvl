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
#include "./serial_logging.h"

SerialLogging::SerialLogging(RVLogLevel newLevel) {
  this->level = newLevel;
  Serial.begin(115200);
}

RVLogLevel SerialLogging::getLogLevel() {
  return this->level;
}

void SerialLogging::print(const char s) {
  Serial.print(s);
}

void SerialLogging::print(const char *s) {
  Serial.print(s);

}

void SerialLogging::println(const char s) {
  Serial.println(s);
}

void SerialLogging::println(const char *s) {
  Serial.println(s);
}

