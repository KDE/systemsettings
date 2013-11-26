/**
 * Copyright (C) 2009 Ben Cooksley <bcooksley@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SystemSettingsApp.h"
#include <kdbusservice.h>

SystemSettingsApp::SystemSettingsApp(int& argc, char* argv[])
    : QApplication(argc, argv)
    , window(0)
{
    setOrganizationDomain("kde.org");
    new KDBusService(KDBusService::Unique, this);
}

SystemSettingsApp::~SystemSettingsApp()
{
}

void SystemSettingsApp::setMainWindow(SettingsBase * main)
{
    window = main;
}

void SystemSettingsApp::quit()
{
    if( window && !window->queryClose() ) {
        return;
    }
    QApplication::quit();
}
