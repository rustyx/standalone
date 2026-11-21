/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2026 Marcin Woźniak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "WaylandClient.h"
#include "WaylandProtocolManager.h"
#include "WlrForeignToplevelManagementV1.h"
#include <QObject>
#include <cstring>
#include <libinputactions/globals.h>

namespace InputActions
{

WaylandClient::WaylandClient(DBusEnvironmentStateProvider &dbusEnvironmentStateProvider)
    : m_dbusEnvironmentStateProvider(dbusEnvironmentStateProvider)
{
}

WaylandClient::~WaylandClient() = default;

bool WaylandClient::initialize()
{
    m_display = wl_display_connect(nullptr);
    if (!m_display) {
        return false;
    }

    m_protocolManager = std::make_unique<WaylandProtocolManager>(wl_display_get_registry(m_display));
    m_protocolManager->addProtocol(std::make_unique<WlrForeignToplevelManagementV1>(m_dbusEnvironmentStateProvider));

    m_displayNotifier = std::make_unique<QSocketNotifier>(wl_display_get_fd(m_display), QSocketNotifier::Read);
    connect(m_displayNotifier.get(), &QSocketNotifier::activated, this, &WaylandClient::onDisplayReadable);
    flush();
    return true;
}

void WaylandClient::onDisplayReadable()
{
    while (wl_display_prepare_read(m_display) != 0) {
        if (wl_display_dispatch_pending(m_display) == -1) {
            checkDisplayError();
            return;
        }
    }

    // Doesn't block, as the socket is readable
    if (wl_display_read_events(m_display) == -1 || wl_display_dispatch_pending(m_display) == -1) {
        checkDisplayError();
        return;
    }
    flush();
}

void WaylandClient::flush()
{
    // Not fatal on EAGAIN (socket buffer full), the remaining requests are sent by the next flush
    if (wl_display_flush(m_display) == -1) {
        checkDisplayError();
    }
}

void WaylandClient::checkDisplayError()
{
    const auto error = wl_display_get_error(m_display);
    if (error == 0) {
        return;
    }

    qWarning(INPUTACTIONS, "Lost connection to the Wayland compositor: %s", strerror(error));
    m_displayNotifier->setEnabled(false);
}

}