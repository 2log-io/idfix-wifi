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

#include "WiFi.h"
#include "WiFiEventHandler.h"
#include "WiFiUtils.h"
#include "auxiliary.h"

extern "C"
{
    #include <esp_log.h>
    #include <esp_wifi.h>
    #include <string.h>
    #include <lwip/sockets.h>
}

namespace
{
	const char*		LOG_TAG = "IDFix::WiFi";
	const uint8_t	MAC_ADDR_LEN = 6;
	const uint8_t	MAC_STRING_LEN = 17;
}

namespace IDFix
{
	namespace WiFi
	{
		WiFi::WiFi(WiFiEventHandler* wiFiEventHandler) : _wiFiEventHandler(wiFiEventHandler)
		{

		}

		bool WiFi::init()
		{
			esp_err_t result;

			if ( _isInitialized == false )
            {
				if ( esp_netif_init() != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "esp_netif_init failed!" );
					return false;
				}

				result = esp_event_loop_create_default();

				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "esp_event_loop_create_default failed - status: %u", result );
					return false;
				}

                result = esp_event_handler_register(WIFI_EVENT,	ESP_EVENT_ANY_ID, WiFi::wifiEventHandlerWrapper, static_cast<void*>(this) );

                if ( result != ESP_OK )
                {
                    ESP_LOGE(LOG_TAG, "init: esp_event_handler_register failed: %u", result);
                    return false;
                }
				#pragma GCC diagnostic push
				#pragma GCC diagnostic ignored "-Wc99-extensions"

				wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();

				#pragma GCC diagnostic pop

				result = esp_wifi_init(&initConfig);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "init: esp_wifi_init failed: %u", result);
					return false;
				}

				result = esp_wifi_set_storage(WIFI_STORAGE_RAM);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "init: esp_wifi_set_storage failed: %u", result);
					return false;
				}

				wifi_country_t countryConfig;

				countryConfig.cc[0] = 'D';
				countryConfig.cc[1] = 'E';
				countryConfig.cc[2] = '\0';

				countryConfig.schan = 1;
				countryConfig.nchan = 13;
				countryConfig.policy = WIFI_COUNTRY_POLICY_MANUAL;

				esp_wifi_set_country(&countryConfig);

				result = esp_wifi_set_mode(WIFI_MODE_NULL);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "connectWPA: esp_wifi_set_mode(WIFI_MODE_NULL) failed: %u", result);
				}

				_isInitialized = true;
				return true;
			}

			return false;
		}

        void WiFi::wifiEventHandler(void* UNUSED(instance), esp_event_base_t eventBase, int32_t eventID, void *eventData)
		{

			if ( eventBase == WIFI_EVENT )
			{
				ESP_LOGI(LOG_TAG, "\033[1;92mWIFI_EVENT: %s\033[0m", WiFiUtils::wiFiEventTypeToString( eventID ) );

				switch (eventID)
				{
					case WIFI_EVENT_STA_START:
					{
						_stationInitialized = true;
						return;
					}

					case WIFI_EVENT_AP_START:
					{
						if ( _wiFiEventHandler != nullptr )
						{
                            tcpip_adapter_ip_info_t		apAdapterInfo;

                            tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &apAdapterInfo);
                            _wiFiEventHandler->accessPointStarted(apAdapterInfo.ip);
						}

						return;
					}

					case WIFI_EVENT_AP_STOP:
					{
						if ( _wiFiEventHandler != nullptr )
						{
							_wiFiEventHandler->accessPointStopped();
						}

						return;
					}

					case WIFI_EVENT_STA_DISCONNECTED:
					{
						if ( _wiFiEventHandler != nullptr )
						{
							_wiFiEventHandler->networkDisconnected();
						}
						return;
					}

					case WIFI_EVENT_AP_STACONNECTED:
					{
						return;
					}

					default:
						ESP_LOGW(LOG_TAG, "WIFI_EVENT not handled: %s", WiFiUtils::wiFiEventTypeToString( eventID ));
						return;
				}

				return;
			}

			if ( eventBase == IP_EVENT )
			{
				ESP_LOGI(LOG_TAG, "\033[1;92mIP_EVENT: %s\033[0m", WiFiUtils::ipEventTypeToString( eventID ) );

				switch (eventID)
				{
					case IP_EVENT_STA_GOT_IP:
					{
						ESP_LOGI(LOG_TAG, "\033[1;92mwifiEventHandler: IP_EVENT: IP_EVENT_STA_GOT_IP\033[0m" );

						ip_event_got_ip_t* event = static_cast<ip_event_got_ip_t*>(eventData);

						#pragma GCC diagnostic push
						#pragma GCC diagnostic ignored "-Wold-style-cast"

						ESP_LOGI(LOG_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip) );

						#pragma GCC diagnostic pop

                        IPInfo ipInfo;

                        ipInfo.ip.addr = event->ip_info.ip.addr;
                        ipInfo.gateway.addr = event->ip_info.gw.addr;
                        ipInfo.netMask.addr = event->ip_info.netmask.addr;

						if ( _wiFiEventHandler != nullptr )
						{
                            _wiFiEventHandler->networkConnected(ipInfo);
						}

						return;
					}

					case IP_EVENT_STA_LOST_IP:
					{
						if ( _wiFiEventHandler != nullptr )
						{
							_wiFiEventHandler->networkDisconnected();
						}
						return;
					}

					default:

						ESP_LOGW(LOG_TAG, "IP_EVENT not handled: %s", WiFiUtils::ipEventTypeToString( eventID ) );
						return;
				}

				return;
			}

		}

        void WiFi::wifiEventHandlerWrapper(void *instance, esp_event_base_t eventBase, int32_t eventID, void *eventData)
        {
            WiFi *objectInstance = static_cast<WiFi*>(instance);

            if ( objectInstance != nullptr )
            {
                objectInstance->wifiEventHandler(instance, eventBase, eventID, eventData);
            }
        }

		bool WiFi::connectWPA(const char *ssid, const char *password)
		{
			esp_err_t		result;
			wifi_config_t	wifiConfigSTA = {};
			wifi_mode_t		currentMode, newMode;

			if ( _isInitialized )
			{
                #ifdef CONFIG_IDF_TARGET_ESP32

                    if ( _stationInterface == nullptr )
                    {
                        _stationInterface = esp_netif_create_default_wifi_sta();

                        if ( _stationInterface == nullptr )
                        {
                            ESP_LOGE(LOG_TAG, "startAP: esp_netif_create_default_wifi_ap failed");
                            return false;
                        }
                    }

                #endif

                if ( _stationEventsRegistered == false )
                {
                    result = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, WiFi::wifiEventHandlerWrapper, static_cast<void*>(this) );
                    if ( result != ESP_OK )
                    {
                        ESP_LOGE(LOG_TAG, "connectWPA: esp_event_handler_register failed: %u", result);
                        return false;
                    }

                    result = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, WiFi::wifiEventHandlerWrapper, static_cast<void*>(this) );
                    if ( result != ESP_OK )
                    {
                        ESP_LOGE(LOG_TAG, "connectWPA: esp_event_handler_register failed: %u", result);
                        return false;
                    }

                    _stationEventsRegistered = true;
                }

				// check if we're already in AP mode and set mode accordingly
				result = esp_wifi_get_mode(&currentMode);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "connectWPA: esp_wifi_get_mode failed: %u", result);
					return false;
				}

				ESP_LOGV(LOG_TAG, "current wifi mode: %d", currentMode);

				switch (currentMode)
				{
					case WIFI_MODE_AP:
					case WIFI_MODE_APSTA:
						newMode = WIFI_MODE_APSTA;
						break;
					default:
						newMode = WIFI_MODE_STA;
				}

				result = esp_wifi_set_mode(newMode);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "connectWPA: esp_wifi_set_mode failed: %u", result);
					return false;
				}

				memset(&wifiConfigSTA, 0, sizeof(wifiConfigSTA) );

				wifiConfigSTA.sta.channel			= 0;
				wifiConfigSTA.sta.scan_method		=	WIFI_ALL_CHANNEL_SCAN;

				#ifdef CONFIG_IDF_TARGET_ESP8266
                    wifiConfigSTA.sta.pmf_cfg.capable	= true;
                    wifiConfigSTA.sta.pmf_cfg.required	= false;
                #endif

				strncpy( reinterpret_cast<char *>(wifiConfigSTA.sta.ssid),		ssid,		sizeof(wifiConfigSTA.sta.ssid) );
				strncpy( reinterpret_cast<char *>(wifiConfigSTA.sta.password),	password,	sizeof(wifiConfigSTA.sta.password) );

				result = esp_wifi_set_config(WIFI_IF_STA, &wifiConfigSTA);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "connectWPA: esp_wifi_set_config failed: %u", result);
					return false;
				}

				result = esp_wifi_start();
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "connectWPA: esp_wifi_start failed: %u", result);
					return false;
				}

				result = esp_wifi_connect();
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "connectWPA: esp_wifi_connect failed: %u", result);
				}

			}
			else
			{
				ESP_LOGE(LOG_TAG, "connectWPA: WiFi is not initialized");
				return false;
			}

			return true;
		}

		bool WiFi::connectWPA(const std::string &ssid, const std::string &password)
		{
			return connectWPA( ssid.c_str(), password.c_str() );
		}

		bool WiFi::startAP(const char *ssid, const char *password)
		{
			esp_err_t		result;
			wifi_config_t	wifiConfigAP = {};
			wifi_mode_t		currentMode, newMode;

			if ( _isInitialized )
			{

                #ifdef CONFIG_IDF_TARGET_ESP32
                    if ( _accessPointInterface != nullptr )
                    {
                        // we're already running an AP, stop this one first
                        return false;
                    }

                    _accessPointInterface = esp_netif_create_default_wifi_ap();

                    if ( _accessPointInterface == nullptr )
                    {
                        ESP_LOGE(LOG_TAG, "startAP: esp_netif_create_default_wifi_ap failed");
                        return false;
                    }
                #endif

				memset(&wifiConfigAP, 0, sizeof(wifiConfigAP) );

				wifiConfigAP.ap.channel = 1;
				wifiConfigAP.ap.beacon_interval = 100;
				wifiConfigAP.ap.max_connection = 1;
				wifiConfigAP.ap.ssid_len = static_cast<uint8_t>( strlen(ssid) );
				strncpy( reinterpret_cast<char *>(wifiConfigAP.ap.ssid),		ssid, sizeof(wifiConfigAP.ap.ssid) );

				if ( strlen(password) == 0 )
				{
					wifiConfigAP.ap.authmode = WIFI_AUTH_OPEN;
				}
				else
				{
					wifiConfigAP.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
					strncpy( reinterpret_cast<char *>(wifiConfigAP.ap.password),	password, sizeof(wifiConfigAP.ap.password) );
				}

				// check if we're already in ST mode and set mode accordingly
				result = esp_wifi_get_mode(&currentMode);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "startAP: esp_wifi_get_mode failed: %u", result);
					return false;
				}

				switch (currentMode)
				{
					case WIFI_MODE_STA:
					case WIFI_MODE_APSTA:
						newMode = WIFI_MODE_APSTA;
						break;
					default:
						newMode = WIFI_MODE_AP;
				}

				ESP_LOGI(LOG_TAG, "current wifi mode: %d", currentMode);
				ESP_LOGI(LOG_TAG, "setting wifi mode: %d", newMode);

				result = esp_wifi_set_mode(newMode);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "startAP: esp_wifi_set_mode failed: %u", result);
					return false;
				}

				result = esp_wifi_set_config(WIFI_IF_AP, &wifiConfigAP);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "startAP: esp_wifi_set_config failed: %u", result);
					return false;
				}

				result = esp_wifi_start();
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "startAP: esp_wifi_start failed: %u", result);
					return false;
				}

			}
			else
			{
				ESP_LOGE(LOG_TAG, "startAP: WiFi is not initialized");
				return false;
			}

			return true;
		}

		bool WiFi::stopAP()
		{
			wifi_mode_t		currentMode, newMode;
			esp_err_t		result;

			// check if we're already in ST mode and set mode accordingly
			result = esp_wifi_get_mode(&currentMode);
			if ( result != ESP_OK )
			{
				ESP_LOGE(LOG_TAG, "stopAP: esp_wifi_get_mode failed: %u", result);
				return false;
			}

			// are we actually in AP / STAP mode?
			if ( (currentMode == WIFI_MODE_AP) || currentMode == WIFI_MODE_APSTA )
			{
				if ( currentMode == WIFI_MODE_APSTA )
				{
					// if we are in APSTA mode, switch to STA only mode
					newMode = WIFI_MODE_STA;
				}
				else
				{
					// else we disable wifi completely
					newMode = 	WIFI_MODE_NULL;
				}

				result = esp_wifi_set_mode(newMode);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "startAP: esp_wifi_set_mode failed: %u", result);
				}

                #ifdef CONFIG_IDF_TARGET_ESP32
                    esp_netif_destroy(_accessPointInterface);
                    _accessPointInterface = nullptr;
                #endif

				if ( newMode != WIFI_MODE_STA )
				{
					// if we're not in station mode now, shutdown wifi
					result = esp_wifi_stop();
					if ( result != ESP_OK )
					{
						ESP_LOGE(LOG_TAG, "stopAP: esp_wifi_stop failed: %u", result);
					}
				}

				return true;
			}

			return false;
		}

		int16_t WiFi::scan(const std::string &ssid, bool showHidden)
		{
			wifi_mode_t		currentMode;
			esp_err_t		result;
			int16_t			returnValue = -1;
			uint16_t		apCount = 0;

			if ( _isInitialized )
			{
				result = esp_wifi_get_mode(&currentMode);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "scan: esp_wifi_get_mode failed: %u", result);
					return -1;
				}

				if ( prepareForScan(currentMode) == false )
				{
					return -1;
				}

				wifi_scan_config_t scanConfig;

				if ( ssid.empty() )
				{
					scanConfig.ssid = nullptr;
				}
				else
				{
					scanConfig.ssid = const_cast<uint8_t*>( reinterpret_cast<const uint8_t*>( ssid.c_str() ) ) ;
					ESP_LOGI(LOG_TAG, "Scanning for ssid: %s", scanConfig.ssid);
				}

				scanConfig.bssid = nullptr;
				scanConfig.channel = 0;
				scanConfig.show_hidden = showHidden;
				scanConfig.scan_type = WIFI_SCAN_TYPE_ACTIVE;
				scanConfig.scan_time.active.min = 100;
				scanConfig.scan_time.active.max = 300;

				result = esp_wifi_scan_start(&scanConfig, true);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "scan: esp_wifi_scan_start failed: %u", result);
					recoverFromScan(currentMode);
					return -1;
				}

				ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&apCount));
				returnValue = static_cast<int16_t>(apCount);

				wifi_ap_record_t	*apList = new wifi_ap_record_t[apCount];
				uint16_t			getAPCount = apCount;

				result = esp_wifi_scan_get_ap_records(&getAPCount, apList);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "scan: esp_wifi_scan_get_ap_records failed: %u", result);

					if ( result == ESP_ERR_WIFI_NOT_INIT )
					{
						ESP_LOGE(LOG_TAG, "ESP_ERR_WIFI_NOT_INIT");
					}
					else if ( result == ESP_ERR_WIFI_NOT_STARTED )
					{
						ESP_LOGE(LOG_TAG, "ESP_ERR_WIFI_NOT_STARTED");
					}
					else if ( result == ESP_ERR_INVALID_ARG )
					{
						ESP_LOGE(LOG_TAG, "ESP_ERR_INVALID_ARG");
					}
					else if ( result == ESP_ERR_NO_MEM )
					{
						ESP_LOGE(LOG_TAG, "ESP_ERR_NO_MEM");
					}
					else
					{
						ESP_LOGE(LOG_TAG, "Unknown esp_wifi_scan_get_ap_records result!");
					}

					returnValue = -1;
				}

				delete[] apList;

				recoverFromScan(currentMode);

			}
			else
			{
				ESP_LOGE(LOG_TAG, "scan: WiFi is not initialized");
				return -1;
			}

			return returnValue;
		}

        std::string WiFi::getStationMACAddress()
		{
			uint8_t		wifiMACAddress[MAC_ADDR_LEN] = {};
			char		wifiMACString[MAC_STRING_LEN + 1];

			if ( esp_read_mac(wifiMACAddress, ESP_MAC_WIFI_STA) != ESP_OK )
			{
				ESP_LOGE(LOG_TAG, "esp_read_mac() error");
				return "";
			}

			// 00:11:22:33:44:55
			snprintf(wifiMACString, MAC_STRING_LEN + 1, "%02X:%02X:%02X:%02X:%02X:%02X", wifiMACAddress[0], wifiMACAddress[1], wifiMACAddress[2], wifiMACAddress[3], wifiMACAddress[4], wifiMACAddress[5]);

            return wifiMACString;
        }

        IPInfo WiFi::getStationIPInfo() const
        {
            tcpip_adapter_ip_info_t		stationAdapterInfo;
            IPInfo ipInfo;

            tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &stationAdapterInfo);

            ipInfo.ip = stationAdapterInfo.ip;
            ipInfo.netMask = stationAdapterInfo.netmask;
            ipInfo.gateway = stationAdapterInfo.gw;

            return ipInfo;
        }

        int8_t WiFi::getRSSILevel() const
        {
            wifi_ap_record_t info;
            esp_wifi_sta_get_ap_info(&info);
            return info.rssi;
        }

		bool WiFi::prepareForScan(wifi_mode_t currentMode)
		{
			esp_err_t	result;

			if ( currentMode ==  WIFI_MODE_NULL )
			{
				result = esp_wifi_set_mode(WIFI_MODE_STA);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "prepareScan: esp_wifi_set_mode failed: %u", result);
					return false;
				}

				// if we're comming from WIFI_MODE_NULL, wifi was not yet started
				result = esp_wifi_start();
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "prepareScan: esp_wifi_start failed: %u", result);
				}
			}
			else if ( currentMode == WIFI_MODE_AP )
			{
				result = esp_wifi_set_mode(WIFI_MODE_APSTA);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "prepareScan: esp_wifi_set_mode failed: %u", result);
					return false;
				}
			}

			return true;
		}

		bool WiFi::recoverFromScan(wifi_mode_t targetMode)
		{
			esp_err_t	result;
			bool		returnValue = true;

			// we only changed state in prepareForScan if mode previously was WIFI_MODE_NULL or WIFI_MODE_AP
			// so we have to recover only in one of this modes
			if ( (targetMode == WIFI_MODE_NULL) || (targetMode == WIFI_MODE_AP) )
			{

				// reset the state prior the scan
				result = esp_wifi_set_mode(targetMode);
				if ( result != ESP_OK )
				{
					ESP_LOGE(LOG_TAG, "scan: esp_wifi_set_mode failed: %u", result);
					returnValue = false;
				}

				if ( targetMode == WIFI_MODE_NULL )
				{
					// if we came from WIFI_MODE_NULL, also disable the wifi finally
					result = esp_wifi_stop();
					if ( result != ESP_OK )
					{
						ESP_LOGE(LOG_TAG, "scan: esp_wifi_stop failed: %u", result);
						returnValue = false;
					}
				}
			}

			return returnValue;
		}

	}
}


