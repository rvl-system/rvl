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

#ifndef RVLLOGGING_H_
#define RVLLOGGING_H_

#include <stdarg.h>

// These are defined such that we can do if(logLevel >= RVLogLevel.Warning) in code
enum class RVLLogLevel {
  Error = 1,
  Info = 2,
  Debug = 3
};

class RVLLoggingInterface {
 public:
  virtual void print(const char *s) = 0;
  virtual void println() = 0;
  virtual void println(const char *s) = 0;
};

class RVLLogging {
 private:
  RVLLogLevel logLevel;
  RVLLoggingInterface* interface;

  void log(const char *s);
  void log(const char *s, va_list argptr);

 public:
  RVLLogging(RVLLoggingInterface* iface, RVLLogLevel level);

  void error(const char *s, ...);
  void info(const char *s, ...);
  void debug(const char *s, ...);
};

#endif  // RVLLOGGING_H_
