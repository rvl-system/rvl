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

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "./RaverLightsMessaging.h"

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

inline void error(const char *s) {
#ifdef LOG_ERROR_ENABLED
  logging->print("[error]: ");
  logging->println(s);
#endif
}

inline void info(const char *s) {
#ifdef LOG_INFO_ENABLED
  logging->print("[info ]: ");
  logging->println(s);
#endif
}

inline void debug(const char *s) {
#ifdef LOG_DEBUG_ENABLED
  logging->print("[debug]: ");
  logging->println(s);
#endif
}

// Variadic versions

template<typename T, typename ...Args>
inline void log(const char *s, T value, Args ...args) {
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
inline void error(const char *s, T value, Args... args) {
#ifdef LOG_ERROR_ENABLED
  logging->print("[error]: ");
  log(s, value, args...);
  logging->println();
#endif
}

template<typename T, typename... Args>
inline void info(const char *s, T value, Args... args) {
#ifdef LOG_INFO_ENABLED
  logging->print("[info ]: ");
  log(s, value, args...);
  logging->println();
#endif
}

template<typename T, typename... Args>
inline void debug(const char *s, T value, Args... args) {
#ifdef LOG_DEBUG_ENABLED
  logging->print("[debug]: ");
  log(s, value, args...);
  logging->println();
#endif
}

}  // namespace Platform

#endif  // PLATFORM_H_
