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

#include "WiFiUtils.h"

extern "C"
{
	#include "esp_wifi_types.h"
	#include "esp_netif.h"
}

namespace IDFix
{
	namespace WiFi
	{
        const char *WiFiUtils::wiFiEventTypeToString(int32_t eventType)
        {
            switch (eventType)
            {
                case WIFI_EVENT_WIFI_READY:				return "WIFI_EVENT_WIFI_READY";
                case WIFI_EVENT_SCAN_DONE:				return "WIFI_EVENT_SCAN_DONE";
                case WIFI_EVENT_STA_START:				return "WIFI_EVENT_STA_START";
                case WIFI_EVENT_STA_STOP:				return "WIFI_EVENT_STA_STOP";
                case WIFI_EVENT_STA_CONNECTED:			return "WIFI_EVENT_STA_CONNECTED";
                case WIFI_EVENT_STA_DISCONNECTED:		return "WIFI_EVENT_STA_DISCONNECTED";
                case WIFI_EVENT_STA_AUTHMODE_CHANGE:	return "WIFI_EVENT_STA_AUTHMODE_CHANGE";
                case WIFI_EVENT_STA_WPS_ER_SUCCESS:		return "WIFI_EVENT_STA_WPS_ER_SUCCESS";
                case WIFI_EVENT_STA_WPS_ER_FAILED:		return "WIFI_EVENT_STA_WPS_ER_FAILED";
                case WIFI_EVENT_STA_WPS_ER_TIMEOUT:		return "WIFI_EVENT_STA_WPS_ER_TIMEOUT";
                case WIFI_EVENT_STA_WPS_ER_PIN:			return "WIFI_EVENT_STA_WPS_ER_PIN";
                case WIFI_EVENT_AP_START:				return "WIFI_EVENT_AP_START";
                case WIFI_EVENT_AP_STOP:				return "WIFI_EVENT_AP_STOP";
                case WIFI_EVENT_AP_STACONNECTED:		return "WIFI_EVENT_AP_STACONNECTED";
                case WIFI_EVENT_AP_STADISCONNECTED:		return "WIFI_EVENT_AP_STADISCONNECTED";
                case WIFI_EVENT_AP_PROBEREQRECVED:		return "WIFI_EVENT_AP_PROBEREQRECVED";
                #ifdef CONFIG_IDF_TARGET_ESP32
                    case WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP:	return "WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP";
                    case WIFI_EVENT_MAX:					return "WIFI_EVENT_MAX";
                #endif
            }

            return "NULL";
        }


		const char *WiFiUtils::ipEventTypeToString(int32_t eventType)
		{
			switch (eventType)
			{
				case IP_EVENT_STA_GOT_IP:			return "IP_EVENT_STA_GOT_IP";
				case IP_EVENT_STA_LOST_IP:			return "IP_EVENT_STA_LOST_IP";
				case IP_EVENT_AP_STAIPASSIGNED:		return "IP_EVENT_AP_STAIPASSIGNED";
				case IP_EVENT_GOT_IP6:				return "IP_EVENT_GOT_IP6";
                #ifdef CONFIG_IDF_TARGET_ESP32
                    case IP_EVENT_ETH_GOT_IP:			return "IP_EVENT_ETH_GOT_IP";
                #endif
			}

			return "NULL";
		}
	}
}
