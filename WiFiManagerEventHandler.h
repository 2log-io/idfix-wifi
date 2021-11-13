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

#ifndef WIFIMANAGEREVENTHANDLER_H
#define WIFIMANAGEREVENTHANDLER_H

#include "WiFiEventHandler.h"
#include <string>

namespace IDFix
{
	namespace WiFi
	{
        /**
         * @brief The WiFiManagerEventHandler class extends the WiFiEventHandler interface to handle additional
         * events provided by the WiFiManager class.
         */
		class WiFiManagerEventHandler : public WiFiEventHandler
		{
			public:

				virtual ~WiFiManagerEventHandler();

                /**
                 * @brief This event is triggered if all configuration
                 * services are up and running and the WiFiManager is waiting
                 * for a configuration client.
                 */
				virtual void configurationStarted();

                /**
                 * @brief This event is triggered after a configuration successfully finished.
                 */
				virtual void configurationFinished();

				 /**
                 * @brief This event is triggered if the configuration services
                 * could not be started.
                 */
				virtual void configurationFailed();

                /**
                 * @brief This event is triggered after the wifi configuration was received from a configuration client.
                 *
                 * @param ssid      the configured Wifi SSID for the device
                 * @param password  the configured Wifi password for the SSID
                 */
				virtual void receivedWiFiConfiguration(const std::string &ssid, const std::string &password);

                /**
                 * @brief This event is triggerd if the configuration client sends some additional configuration parameter.
                 *
                 * Configuration clients can provide additional arbitrary device configurations during the configuration.
                 * This parameters are provided as individual parameter/value keys by this event.
                 *
                 * @param param     the parameter name
                 * @param value     the parameter string value
                 */
				virtual void receivedConfigurationParameter(const std::string &param, const std::string &value);

				/**
                 * @brief This event is triggerd if the configuration client sends some additional configuration parameter.
                 *
                 * Configuration clients can provide additional arbitrary device configurations during the configuration.
                 * This parameters are provided as individual parameter/value keys by this event.
                 *
                 * @param param     the parameter name
                 * @param value     the parameter double value
                 */
				virtual void receivedConfigurationParameter(const std::string &param, const double value);

				/**
                 * @brief This event is triggerd if the configuration client sends some additional configuration parameter.
                 *
                 * Configuration clients can provide additional arbitrary device configurations during the configuration.
                 * This parameters are provided as individual parameter/value keys by this event.
                 *
                 * @param param     the parameter name
                 * @param value     the parameter bool value
                 */
				virtual void receivedConfigurationParameter(const std::string &param, const bool value);
		};
	}
}

#endif
