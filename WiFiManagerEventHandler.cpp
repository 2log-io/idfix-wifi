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

#include "WiFiManagerEventHandler.h"
#include "auxiliary.h"

namespace IDFix
{
	namespace WiFi
	{

		WiFiManagerEventHandler::~WiFiManagerEventHandler()
		{

		}

		void WiFiManagerEventHandler::configurationStarted()
		{

		}

		void WiFiManagerEventHandler::configurationFinished()
		{

		}

		void WiFiManagerEventHandler::configurationFailed()
		{

		}

		void WiFiManagerEventHandler::receivedWiFiConfiguration(const std::string &UNUSED(ssid), const std::string &UNUSED(password) )
		{

		}

		void WiFiManagerEventHandler::receivedConfigurationParameter(const std::string &UNUSED(param), const std::string &UNUSED(value) )
		{

        }

        void WiFiManagerEventHandler::receivedConfigurationParameter(const std::string &UNUSED(param), const double UNUSED(value) )
        {

        }

        void WiFiManagerEventHandler::receivedConfigurationParameter(const std::string &UNUSED(param), const bool UNUSED(value) )
        {

        }

	}
}
