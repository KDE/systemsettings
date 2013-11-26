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

/**
 * Any changes to this header file need to have the following command executed afterwards to regenerate the dbus interface
 * qdbuscpp2xml SystemSettingsApp.h -o org.kde.systemsettings.xml
 */

#ifndef SYSTEMSETTINGSAPP_H
#define SYSTEMSETTINGSAPP_H

#include <QApplication>

#include "SettingsBase.h"

class SystemSettingsApp : public QApplication
{
    Q_OBJECT

public:
   SystemSettingsApp(int& argc, char* argv[]);
   ~SystemSettingsApp();

   void setMainWindow(SettingsBase * main);

public Q_SLOTS:
   Q_SCRIPTABLE void quit();

private:
   SettingsBase * window; 
};

#endif
