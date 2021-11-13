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

#ifndef WIFI_H
#define WIFI_H

extern "C"
{
    #include <esp_event.h>
    #include <esp_netif.h>
    #include <arpa/inet.h>
    #include <lwip/sockets.h>
}

#include <string>

namespace IDFix
{
	namespace WiFi
	{
		class WiFiEventHandler;

        struct IPInfo
        {
            ip4_addr_t ip;
            ip4_addr_t netMask;
            ip4_addr_t gateway;

            static std::string ipToString(const ip4_addr_t &ip)
            {
                char ipAddress[INET_ADDRSTRLEN];
                const char* addr = inet_ntop(AF_INET, &ip, ipAddress, INET_ADDRSTRLEN);

                if ( addr == nullptr )
                {
                    return std::string();
                }

                return addr;
            }
        };

        /**
         * @brief The WiFi class allows to control the WIFI adapter of the device
         */
		class WiFi
		{
			public:
                                    WiFi(WiFiEventHandler* wiFiEventHandler);

                /**
                 * @brief   Initialize the WIFI adapter
                 *
                 * @return  \c false if adapter could not be initialized
                 */
				bool				init(void);

                /**
                 * @brief Connect to a WPA protected WIFI
                 *
                 * @param ssid      the SSID of the WIFI
                 * @param password  the password for the WIFI
                 *
                 * @return  \c false if the WIFI station mode coud not be started
                 */
				bool				connectWPA(const char *ssid, const char *password);
				bool				connectWPA(const std::string &ssid, const std::string &password);

                /**
                 * @brief Create an unprotected access point
                 *
                 * @param ssid          the SSID to use for the acces point
                 * @param password      the password to use for the acces point
                 *
                 * @return  \c false if access point could not be started
                 */
				bool				startAP(const char *ssid, const char *password);

                /**
                 * @brief Shut down a created access point
                 *
                 * @return \c false if no access point is started
                 */
				bool				stopAP(void);

                /**
                 * @brief Scan for all available or a specified SSID and get the number of available networks
                 *
                 * @param ssid          only scan for the specified SSID
                 * @param showHidden    also include hidden networks
                 *
                 * @return              the number of found networks
                 */
				int16_t				scan(const std::string &ssid = "", bool showHidden = true);

                /**
                 * @brief Get the MAC address of the device
                 *
                 * @return the MAC address as string representation
                 */
                static std::string	getStationMACAddress(void);

                /**
                 * @brief Get the current IP addresses of the station
                 *
                 * @return the current IP addresses
                 */
                IPInfo              getStationIPInfo() const;

                /**
                 * @brief Get the current RSSI level of the connected WIFI
                 *
                 * @return the current RSSI level
                 */
                int8_t              getRSSILevel() const;

            private:

                void				wifiEventHandler(		void* instance, esp_event_base_t eventBase, int32_t eventID, void* eventData);
                static void			wifiEventHandlerWrapper(void* instance, esp_event_base_t eventBase, int32_t eventID, void* eventData);

            protected:

                /**
                 * @brief Set the correct wifi mode for scanning according to the current mode
                 *
                 * @param currentMode   the current set WIFI mode
                 *
                 * @return      \c false if mode could not be set for scanning
                 */
				bool				prepareForScan(wifi_mode_t currentMode);

                /**
                 * @brief Recover the correct mode which was set before scanning
                 *
                 * @param targetMode    the mode which was set before scanning
                 *
                 * @return      \c false if targetMode could not be set
                 */
				bool				recoverFromScan(wifi_mode_t targetMode);


				WiFiEventHandler*	_wiFiEventHandler = { nullptr };
				bool				_isInitialized = { false };
				bool				_stationInitialized = { false };
                bool                _stationEventsRegistered = { false };

                #ifdef CONFIG_IDF_TARGET_ESP32
                    esp_netif_t*		_stationInterface = { nullptr };
                    esp_netif_t*		_accessPointInterface = { nullptr };
                #endif

		};
	}
}

#endif
