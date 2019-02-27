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

#ifndef SERIAL_LOGGING_H_
#define SERIAL_LOGGING_H_

#include <stdint.h>
#include <Arduino.h>
#include <RaverLightsMessaging.h>

#define SERVER_PORT 4978

class SerialLogging : public RVLoggingInterface {
 private:
  RVLogLevel level;

 public:
  explicit SerialLogging(RVLogLevel level);

  RVLogLevel getLogLevel();
  void print(const char s);
  void print(const char *s);
  void println();
  void println(const char s);
  void println(const char *s);
};

#endif  // SERIAL_LOGGING_H_
