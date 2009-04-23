/**
 * Copyright (C) 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>
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

#ifndef KCONTROLAPP_H
#define KCONTROLAPP_H

#include <KUniqueApplication>

#include "SettingsBase.h"

class KControlApp : public KUniqueApplication
{
    Q_OBJECT

public:
   KControlApp();
   ~KControlApp();

   void setMainWindow(SettingsBase * main);

public Q_SLOTS:
   Q_SCRIPTABLE void quit();

private:
   SettingsBase * window; 
};

#endif
