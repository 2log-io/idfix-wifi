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

#include "WiFiManager.h"

#include "WiFiManagerEventHandler.h"
#include "TLSServer.h"
#include "TLSSocket.h"
#include "auxiliary.h"

extern "C"
{
	#include <esp_log.h>
}

namespace
{
	const char* LOG_TAG = "IDFix::WiFiManager";
	const char* INVALID_MESSAGE		= "{ \"error\": \"invalid message\"}\r\n";
	const char* INVALID_COMMAND		= "{ \"error\": \"invalid command\"}\r\n";
    const char* SETCONFIG_ACK_MSG	= "{ \"cmd\":\"setconfig\",\"status\":1}\r\n";
}

namespace IDFix
{
	namespace WiFi
	{

		WiFiManager::WiFiManager(WiFiManagerEventHandler *eventHandler)
			: WiFi(this), _managerEventHandler(eventHandler)
		{

		}

		bool WiFiManager::startConfiguration(const std::string &ssid, const std::string &password)
		{
			if ( _configState != ConfigurationState::Inactive )
			{
				// configuration already running
				return false;
			}

			int16_t	scanResult;

			scanResult = scan(ssid);
			if ( scanResult < 0 )
			{
				ESP_LOGE(LOG_TAG, "Faild to scan for existing configuration network");
				return false;
			}

			if ( scanResult > 0 )
			{
				ESP_LOGE(LOG_TAG, "There's already a device in config mode!");
				return false;
			}

			_configState = ConfigurationState::Starting;

			return startAP( ssid.c_str(), password.c_str() );
		}

		void WiFiManager::setCertificate(const unsigned char *cert, long certLength)
		{
			_managerCert = cert;
			_managerCertLength = certLength;
		}

		void WiFiManager::setPrivateKey(const unsigned char *key, long keyLength)
		{
			_managerKey = key;
			_managerKeyLength = keyLength;
		}

		void WiFiManager::addConfigDeviceParameter(const std::string &parameter, const std::string &value)
		{
			_deviceParameter.insert( StringMap::value_type(parameter, value) );
		}

        void WiFiManager::networkConnected(const IPInfo &ipInfo)
		{
			if ( _managerEventHandler != nullptr )
			{
				_managerEventHandler->networkConnected(ipInfo);
			}
		}

		void WiFiManager::networkDisconnected()
		{
			if ( _managerEventHandler != nullptr )
			{
				_managerEventHandler->networkDisconnected();
			}
		}

		void WiFiManager::accessPointStarted(ip4_addr_t accessPointIPAddress)
		{
			if ( _configState == ConfigurationState::Starting )
			{
				// if we have started an AP due to a configuration, prepare configuration server

				ESP_LOGI(LOG_TAG, "accessPointStarted for config with IP:" IPSTR, IP2STR(&accessPointIPAddress) );

				_configurationServer = new Protocols::TLSServer(this);

				if ( ! startConfigurationServer() )
                {
                    delete _configurationServer;
                    _configurationServer = nullptr;

                    _configState = ConfigurationState::Inactive;

                    if ( _managerEventHandler != nullptr )
                    {
                        _managerEventHandler->configurationFailed();
                    }

                    return;
                }

				_dnsResponder = new Protocols::SimpleDNSResponder();

				if ( _dnsResponder->start(accessPointIPAddress, 53) != 0 )
				{
                    delete _dnsResponder;
                    _dnsResponder = nullptr;

                    _configurationServer->shutdown();

                    _configState = ConfigurationState::Inactive;

                    if ( _managerEventHandler != nullptr )
                    {
                        _managerEventHandler->configurationFailed();
                    }

                    return;
				}

				_configState = ConfigurationState::Pending;

				if ( _managerEventHandler != nullptr )
				{
					_managerEventHandler->configurationStarted();
				}
			}
			else
			{
				// if access point not started due to configuration, just forward event
				if ( _managerEventHandler != nullptr )
				{
					_managerEventHandler->accessPointStarted(accessPointIPAddress);
				}
			}

		}

		bool WiFiManager::startConfigurationServer()
        {
            if ( ! _configurationServer->init() )
            {
                ESP_LOGE(LOG_TAG, "Failed to init TLSServer at file %s:%d.", __FILE__, __LINE__);
                return false;
            }

            if ( ! _configurationServer->setCertificate(_managerCert, _managerCertLength) )
            {
                ESP_LOGE(LOG_TAG, "Failed to set certificate at file %s:%d.", __FILE__, __LINE__);
                return false;
            }

            if ( ! _configurationServer->setPrivateKey(_managerKey, _managerKeyLength) )
            {
                ESP_LOGE(LOG_TAG, "Failed to set private key at file %s:%d.", __FILE__, __LINE__);
                return false;
            }

            if ( ! _configurationServer->listen(8443) )
            {
                ESP_LOGE(LOG_TAG, "Failed to set TLSServer to listening at file %s:%d.", __FILE__, __LINE__);
                return false;
            }

            ESP_LOGI(LOG_TAG, "TLSServer listening on port 8443");
            return true;
        }

		void WiFiManager::accessPointStopped()
		{
			if ( _managerEventHandler != nullptr )
			{
				_managerEventHandler->accessPointStopped();
			}
		}

		void WiFiManager::tlsNewConnection(TLSSocket_weakPtr tlsSocket)
		{
			ESP_LOGI(LOG_TAG, "New config device connected!");
			TLSSocket_sharedPtr	sharedSocket = tlsSocket.lock();

			if ( !sharedSocket )
			{
				ESP_LOGE(LOG_TAG, "Failed to obtain sharedPtr in tlsNewConnection");
				return;
			}

			if ( _configState == ConfigurationState::Pending )
			{
				_configState = ConfigurationState::Running;
				_configSocket = tlsSocket;
				sharedSocket->setEventHandler(this);
			}
			else
			{
				ESP_LOGW(LOG_TAG, "Incomming connection in unexpected state!");
                sharedSocket->write("Configuration already running... Bye!");
                sharedSocket->close();
			}

		}

		void WiFiManager::socketBytesReceived(TLSSocket& tlsSocket, ByteArray &bytes)
		{
			ESP_LOGV(LOG_TAG, "socketBytesReceived: %s", reinterpret_cast<char*>( bytes.data() ) );

			cJSON *jsonMessage = cJSON_Parse( reinterpret_cast<char*>( bytes.data() ));

			if ( jsonMessage == nullptr || ! cJSON_IsObject(jsonMessage) )
			{
				ESP_LOGE(LOG_TAG, "Invalid json message received");
                tlsSocket.write(INVALID_MESSAGE);
				return;
			}

			if ( _configState == ConfigurationState::Running && _managerEventHandler != nullptr )
			{
				handleJSONConfigMessage(jsonMessage);
			}

			cJSON_Delete(jsonMessage);
		}

		void WiFiManager::socketDisconnected(TLSSocket& UNUSED(tlsSocket) )
		{
            if ( _configState == ConfigurationState::Running )
            {
                // if configuration was not finished, reset state to pending
                _configState = ConfigurationState::Pending;
            }

			// reset weak pointer to socket to release all memory
			_configSocket.reset();
		}

		void WiFiManager::handleJSONConfigMessage(cJSON *root)
		{
			TLSSocket_sharedPtr	sharedSocket = _configSocket.lock();

			if ( !sharedSocket )
			{
				_configSocket.reset();
				_configState = ConfigurationState::Pending;

				ESP_LOGE(LOG_TAG, "Failed to obtain sharedPtr in handleJSONConfigMessage");
				return;
			}

			cJSON *commandObject = cJSON_GetObjectItemCaseSensitive(root, "cmd");

			if ( ! ( cJSON_IsString(commandObject) && commandObject->valuestring != nullptr ) )
			{
				ESP_LOGE(LOG_TAG, "Json message does not contain a command");
                sharedSocket->write(INVALID_MESSAGE);
				return;
			}

			std::string command = commandObject->valuestring;

			if ( command == "hi" )
			{
				sendConfigWelcomeMessage();
			}
			else if ( command == "setconfig" )
			{
				handleSetConfigMessage(root);
                sharedSocket->write(SETCONFIG_ACK_MSG);

                finishConfiguration();
			}
			else
			{
                sharedSocket->write(INVALID_COMMAND);
			}

		}

		bool WiFiManager::sendConfigWelcomeMessage()
		{
			cJSON *root = cJSON_CreateObject();
			cJSON *device = cJSON_CreateObject();
			bool result = true;

			if (cJSON_AddStringToObject(root, "cmd", "welcome") == nullptr)
			{
				ESP_LOGE(LOG_TAG, "cJSON_AddStringToObject failed");
				cJSON_Delete(root);
				return false;
			}

			for ( const StringMap::value_type& parameter : _deviceParameter )
			{
				if (cJSON_AddStringToObject(device, parameter.first.c_str(), parameter.second.c_str() ) == nullptr)
				{
					ESP_LOGE(LOG_TAG, "cJSON_AddStringToObject device failed: %s", parameter.first.c_str() );
				}
			}

			cJSON_AddItemToObject(root, "device", device);

			char* welcomeMessage = cJSON_Print(root);

			TLSSocket_sharedPtr	sharedSocket = _configSocket.lock();

			if ( sharedSocket && welcomeMessage != nullptr )
			{
                sharedSocket->write(welcomeMessage);
                sharedSocket->write("\r\n");
			}
			else
			{
				ESP_LOGE(LOG_TAG, "Failed to send welcome message");
				result = false;

				if ( !sharedSocket )
				{
					_configSocket.reset();
					_configState = ConfigurationState::Pending;
				}
			}

			free(welcomeMessage);
			cJSON_Delete(root);

			return result;
		}

		void WiFiManager::handleSetConfigMessage(cJSON *root)
		{
			std::string ssid, password;

			cJSON *json = cJSON_GetObjectItemCaseSensitive(root, "ssid");
			if ( json->valuestring != nullptr )
			{
				ssid = json->valuestring;
			}

			json = cJSON_GetObjectItemCaseSensitive(root, "pass");
			if ( json->valuestring != nullptr )
			{
				password = json->valuestring;
			}

			_managerEventHandler->receivedWiFiConfiguration(ssid, password);

			cJSON *extConfig = cJSON_GetObjectItemCaseSensitive(root, "extconfig");
			cJSON* config;

			if ( cJSON_IsObject(extConfig) )
			{
				std::string configStr, valueStr;
				bool valueBool;
				config = extConfig->child;
				while ( config )
				{
					if ( cJSON_IsString(config) )
					{
						configStr = config->string;
						valueStr = config->valuestring;

						_managerEventHandler->receivedConfigurationParameter(configStr, valueStr);
					}
					else if ( cJSON_IsNumber(config) )
					{
                        configStr = config->string;

						_managerEventHandler->receivedConfigurationParameter(configStr, config->valuedouble);
					}
					else if ( cJSON_IsBool(config) )
					{
                        configStr = config->string;
                        if ( cJSON_IsTrue(config) )
                        {
                            valueBool = true;
                        }
                        else
                        {
                            valueBool = false;
                        }

						_managerEventHandler->receivedConfigurationParameter(configStr, valueBool);
					}

					config = config->next;
				}
			}
		}

		void WiFiManager::finishConfiguration()
		{
			_configState = ConfigurationState::Inactive;

			TLSSocket_sharedPtr	sharedSocket = _configSocket.lock();

			if ( !sharedSocket )
			{
				_configSocket.reset();
			}
			else
			{
                sharedSocket->close();
			}

			if ( _managerEventHandler != nullptr )
			{
				_managerEventHandler->configurationFinished();
			}

            _configurationServer->shutdown();
            _dnsResponder->stop();

            delete _dnsResponder;
            _dnsResponder = nullptr;

            stopAP();
        }


	}
}
