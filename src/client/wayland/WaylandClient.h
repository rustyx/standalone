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

#pragma once

#include <QObject>
#include <QSocketNotifier>
#include <memory>
#include <wayland-client.h>

namespace InputActions
{

class DBusEnvironmentStateProvider;
class WaylandProtocolManager;

class WaylandClient : public QObject
{
public:
    WaylandClient(DBusEnvironmentStateProvider &dbusEnvironmentStateProvider);
    ~WaylandClient() override;

    /**
     * @returns Whether the Wayland client was successfully initialized.
     */
    bool initialize();

private slots:
    void onDisplayReadable();

private:
    /**
     * Sends queued requests. Must be called after anything that may send requests, as nothing else does.
     */
    void flush();
    /**
     * Stops processing the display if a fatal error has occurred, as its socket would otherwise keep waking up the event loop.
     */
    void checkDisplayError();

    wl_display *m_display;
    /**
     * Events are read only when available, so that an unresponsive compositor can't block the main thread.
     */
    std::unique_ptr<QSocketNotifier> m_displayNotifier;
    std::unique_ptr<WaylandProtocolManager> m_protocolManager;

    DBusEnvironmentStateProvider &m_dbusEnvironmentStateProvider;
};

}