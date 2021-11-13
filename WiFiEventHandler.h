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

#ifndef WIFIEVENTHANDLER_H
#define WIFIEVENTHANDLER_H

#include "WiFi.h"

extern "C"
{
    #include "esp_netif.h"
}

namespace IDFix
{
	namespace WiFi
	{
        /**
         * @brief The WiFiEventHandler class provides an interface to handle WIFI events
         */
		class WiFiEventHandler
		{
			public:

				virtual			~WiFiEventHandler();

                /**
                 * @brief This event is triggered if the WIFI adapter successfully was connected to a WIFI
                 * @param ipInfo    the configured IP addresses for the WIFI
                 */
                virtual void	networkConnected(const IPInfo &ipInfo);

                /**
                 * @brief This event is triggered if the WIFI adapter could not establish a connection to a WIFI or a connected WIFI was disconnected.
                 */
				virtual void	networkDisconnected(void);

                /**
                 * @brief This event is triggered if an access point was successfully started
                 * @param the IP address of the started access point
                 */
				virtual void	accessPointStarted(ip4_addr_t accessPointIPAddress);

                /**
                 * @brief This event is triggered if the access point is finally stopped.
                 */
				virtual void	accessPointStopped(void);
		};
	}
}

#endif
