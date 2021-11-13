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

#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include "WiFiEventHandler.h"
#include "WiFi.h"
#include "TLSServerEventHandler.h"
#include "TLSSocketEventHandler.h"
#include "SimpleDNSResponder.h"
#include <string>
#include <map>
#include "auxiliary.h"

extern "C"
{
	#include <cJSON.h>
}

typedef std::map< std::string, std::string > StringMap;

using namespace IDFix::Protocols;

namespace IDFix
{
	namespace Protocols
	{
		class TLSServer;
		class TLSSocket;

		DeclarePointers(TLSSocket);
	}

	namespace WiFi
	{
		class WiFiManagerEventHandler;

		class WiFiManager : public WiFiEventHandler, public WiFi, public TLSServerEventHandler, public TLSSocketEventHandler
		{

			public:

								WiFiManager(WiFiManagerEventHandler* eventHandler = nullptr);

                /**
                 * @brief Set the X.509 certificate for the configuration TLS server
                 *
                 * The certificate is used together with the private key to provide the
                 * WIFIManagers's identity to the configuration client.
                 *
                 * @param cert          the X.509 certificate in PEM format as null-terminated string.
                 * @param certLength    the length of the certificate key in bytes.
                 */
				void			setCertificate(const unsigned char *cert, long certLength);

                /**
                 * @brief Set the private key of the certificate for the configuration TLS server
                 *
                 * The private key and the certificate are used by the WIFIManagers to provide
                 * it's identity to the configuration client.
                 *
                 * @param key           the private key in PEM format as null-terminated string.
                 * @param keyLength     the length of the private key in bytes.
                 */
				void			setPrivateKey(const unsigned char *key, long keyLength);

                /**
                 * @brief Add a device configuration parameter sent to the configuration client in the welcome message.
                 *
                 * @param parameter     a name for the configuration parameter
                 * @param value         the value for the configuration parameter
                 */
				void			addConfigDeviceParameter(const std::string& parameter, const std::string& value);

                /**
                 * @brief Start a device configuration and wait for configuration clients.
                 *
                 * @param ssid      the SSID to use for the configuration WIFI
                 * @param password  an optional password for the configuration WIFI
                 * @return
                 */
				bool			startConfiguration(const std::string &ssid, const std::string &password = "");

			protected:

				enum class ConfigurationState
				{
					Inactive,
					Starting,
					Pending,
					Running
				};

                /**
                 * @brief The WiFiManager acts as WiFiEventHandler for the WiFi base class.
                 *
                 * All WiFI events are catched by the WiFiManager and will be dispatched to the
                 * according WiFiManagerEventHandler if appropriate.
                 */
                virtual void	networkConnected(const IPInfo &ipInfo) override;

                /**
                 * @brief The WiFiManager acts as WiFiEventHandler for the WiFi base class.
                 *
                 * All WiFI events are catched by the WiFiManager and will be dispatched to the
                 * according WiFiManagerEventHandler if appropriate.
                 */
				virtual void	networkDisconnected(void) override;

                /**
                 * @brief The WiFiManager acts as WiFiEventHandler for the WiFi base class.
                 *
                 * All WiFI events are catched by the WiFiManager and will be dispatched to the
                 * according WiFiManagerEventHandler if appropriate.
                 */
				virtual void	accessPointStarted(ip4_addr_t accessPointIPAddress) override;

				/**
                 * @brief The WiFiManager acts as WiFiEventHandler for the WiFi base class.
                 *
                 * All WiFI events are catched by the WiFiManager and will be dispatched to the
                 * according WiFiManagerEventHandler if appropriate.
                 */
				virtual void	accessPointStopped(void) override;

                /**
                 * @brief Handle an incomming TLS connection from a configuration client
                 * @param socket the TLS client socket
                 */
				virtual void	tlsNewConnection(TLSSocket_weakPtr socket) override;

                /**
                 * @brief Handle incoming messages from a configuration client
                 * @param tlsSocket the TLS client socket
                 * @param bytes     the received bytes
                 */
				virtual void	socketBytesReceived(TLSSocket& tlsSocket, ByteArray &bytes) override;

                /**
                 * @brief Handles a disconnect from a configuration client
                 * @param tlsSocket the closed TLS client socket
                 */
				virtual void	socketDisconnected(TLSSocket& tlsSocket) override;


				void			handleJSONConfigMessage(cJSON *root);
				bool			sendConfigWelcomeMessage(void);
				void			handleSetConfigMessage(cJSON *root);

                /**
                 * @brief Stop the configuration mode and shut down all services.
                 */
				void			finishConfiguration(void);

				bool            startConfigurationServer(void);

			protected:

				WiFiManagerEventHandler*	_managerEventHandler;
				const unsigned char*		_managerCert = { nullptr };
				long						_managerCertLength = { 0 };
				const unsigned char*		_managerKey = { nullptr };
				long						_managerKeyLength = { 0 };

				StringMap					_deviceParameter = {};

				TLSServer*					_configurationServer = { nullptr };
				SimpleDNSResponder*			_dnsResponder = { nullptr };
				TLSSocket_weakPtr			_configSocket;

				ConfigurationState			_configState = { ConfigurationState::Inactive };

		};
	}
}

#endif
