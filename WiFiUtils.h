/*   2log.io
 *   Copyright (C) 2021 - 2log.io | mail@2log.io,  sascha@2log.io
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WIFIUTILS_H
#define WIFIUTILS_H

extern "C"
{
    #include "stdint.h"
}

namespace IDFix
{
    namespace WiFi
    {
        /**
         * @brief The WiFiUtils class provides a debugging helper to convert WIFI event consts to a string representation
         */
        class WiFiUtils
        {
            public:

                static const char* wiFiEventTypeToString(int32_t eventType);
                static const char* ipEventTypeToString(int32_t eventType);
        };
    }
}

#endif
