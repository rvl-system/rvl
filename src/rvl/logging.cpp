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
#include <string.h>

namespace rvl {

LogLevel logLevel = LogLevel::Debug;

void setLogLevel(LogLevel newLevel) {
  logLevel = newLevel;
}

void log(const char* s) {
  Platform::system->print(s);
}

void log(const char* s, va_list argptr) {
  int bufferLength = strlen(s) * 3;
  char* str = new char[bufferLength];
  vsnprintf(str, bufferLength, s, argptr);
  Platform::system->print(str);
  delete[] str;
}

void error(const char* s, ...) {
  if (logLevel >= LogLevel::Error) {
    Platform::system->print("[error]: ");
    va_list argptr;
    va_start(argptr, s);
    log(s, argptr);
    Platform::system->println("");
  }
}

void info(const char* s, ...) {
  if (logLevel >= LogLevel::Info) {
    Platform::system->print("[info ]: ");
    va_list argptr;
    va_start(argptr, s);
    log(s, argptr);
    Platform::system->println("");
  }
}

void debug(const char* s, ...) {
  if (logLevel >= LogLevel::Debug) {
    Platform::system->print("[debug]: ");
    va_list argptr;
    va_start(argptr, s);
    log(s, argptr);
    Platform::system->println("");
  }
}

} // namespace rvl
