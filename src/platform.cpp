/*
Copyright (c) 2018 Bryan Hughes <bryan@nebri.us>

This file is part of Raver Lights.

Raver Lights is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Raver Lights is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Raver Lights.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "./RaverLightsMessaging.h"
#include "./platform.h"

namespace Platform {

RVTransportInterface* transport;
RVLoggingInterface* logging;
RVPlatformInterface* platform;

// Base case versions

void init(RVPlatformInterface* newPlatform, RVTransportInterface* newTransport, RVLoggingInterface* newLogging) {
  platform = newPlatform;
  transport = newTransport;
  logging = newLogging;
}

inline void log(const char *s) {
  logging->print(s);
}

void error(const char *s) {
  RVLogLevel logLevel = Platform::logging->getLogLevel();
  if (logLevel >= RVLogLevel::Error) {
    logging->print("[error]: ");
    logging->println(s);
  }
}

void info(const char *s) {
  RVLogLevel logLevel = Platform::logging->getLogLevel();
  if (logLevel >= RVLogLevel::Info) {
    logging->print("[info ]: ");
    logging->println(s);
  }
}

void debug(const char *s) {
  RVLogLevel logLevel = Platform::logging->getLogLevel();
  if (logLevel >= RVLogLevel::Debug) {
    logging->print("[debug]: ");
    logging->println(s);
  }
}

// Variadic versions

template<typename T, typename ...Args>
void log(const char *s, T value, Args ...args) {
  while (*s) {
    if (*s == '%' && *(++s) != '%') {
      logging->print(value);
      s++;
      log(s, args...);
      return;
    }
    logging->print(*(s++));
  }
}

template<typename T, typename... Args>
void error(const char *s, T value, Args... args) {
  RVLogLevel logLevel = logging->getLogLevel();
  if (logLevel >= RVLogLevel::Error) {
    logging->print("[error]: ");
    log(s, value, args...);
    logging->println();
  }
}

template<typename T, typename... Args>
void info(const char *s, T value, Args... args) {
  RVLogLevel logLevel = logging->getLogLevel();
  if (logLevel >= RVLogLevel::Info) {
    logging->print("[info ]: ");
    log(s, value, args...);
    logging->println();
  }
}

template<typename T, typename... Args>
void debug(const char *s, T value, Args... args) {
  RVLogLevel logLevel = logging->getLogLevel();
  if (logLevel >= RVLogLevel::Debug) {
    logging->print("[debug ]: ");
    log(s, value, args...);
    logging->println();
  }
}

}  // namespace Platform
