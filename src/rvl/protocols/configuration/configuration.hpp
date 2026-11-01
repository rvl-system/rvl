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

#ifndef RVL_PROTOCOLS_CONFIGURATION_CONFIGURATION_H_
#define RVL_PROTOCOLS_CONFIGURATION_CONFIGURATION_H_

#include <stdint.h>

namespace rvl {

namespace ProtocolConfiguration {

// Each answers the source with this node's status
void parseQuery(uint8_t source, uint16_t length);
void parseConfigure(uint8_t source, uint16_t length);
void parseIdentify(uint8_t source, uint16_t length);

} // namespace ProtocolConfiguration

} // namespace rvl

#endif // RVL_PROTOCOLS_CONFIGURATION_CONFIGURATION_H_
