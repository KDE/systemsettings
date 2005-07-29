/**
 * This file is part of the System Settings package
 * Copyright (C) 2005 Benjamin C Meyer
 *                    <ben+systempreferences at meyerhome dot net>
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
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "mainwindow.h"
#include "version.h"

static KCmdLineOptions options[] =
{
	{ "menu <argument>", I18N_NOOP("Menu file"), "systemsettings" },
	{ "e", 0, 0 },
	{ "noembed", I18N_NOOP("Embed windows"), 0 },
	KCmdLineLastOption
};

int main( int argc, char *argv[] )
{
	// About data
  KAboutData aboutData("systemsettings", I18N_NOOP("System Settings"),
	  SYSTEM_SETTINGS_VERSION, I18N_NOOP("System Settings"),
	  KAboutData::License_LGPL, "(c) 2005, Benjamin C. Meyer", 0, 0);
  aboutData.addAuthor("Benjamin C. Meyer", I18N_NOOP("Author & Maintainer"),
	  "ben+systempreferences@meyerhome.net");
  KCmdLineArgs::init(argc, argv, &aboutData);

	// Tell which options are supported
  KCmdLineArgs::addCmdLineOptions( options );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	// Launch
  KApplication application(argc, argv);

	MainWindow *mainWindow = new MainWindow(args->isSet("embed"), args->getOption("menu"));
	application.setMainWidget( mainWindow );
	mainWindow->show();

	return application.exec();
}

