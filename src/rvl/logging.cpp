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

#include "./rvl/logging.hpp"
#include "./rvl.hpp"
#include "./rvl/platform.hpp"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

// Longer lines are cut short
#define MAX_LOG_LINE_LENGTH 256

namespace rvl {

LogLevel logLevel = LogLevel::Debug;

void setLogLevel(LogLevel newLevel) {
  logLevel = newLevel;
}

void log(const char* s, va_list argptr) {
  char line[MAX_LOG_LINE_LENGTH];
  vsnprintf(line, sizeof(line), s, argptr);
  Platform::system->print(line);
}

void error(const char* s, ...) {
  if (logLevel >= LogLevel::Error) {
    Platform::system->print("[error]: ");
    va_list argptr;
    va_start(argptr, s);
    log(s, argptr);
    va_end(argptr);
    Platform::system->println("");
  }
}

void info(const char* s, ...) {
  if (logLevel >= LogLevel::Info) {
    Platform::system->print("[info ]: ");
    va_list argptr;
    va_start(argptr, s);
    log(s, argptr);
    va_end(argptr);
    Platform::system->println("");
  }
}

void debug(const char* s, ...) {
  if (logLevel >= LogLevel::Debug) {
    Platform::system->print("[debug]: ");
    va_list argptr;
    va_start(argptr, s);
    log(s, argptr);
    va_end(argptr);
    Platform::system->println("");
  }
}

} // namespace rvl
