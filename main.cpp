/**
 * This file is part of the System Preferences package
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "mainwindow.h"
#include "version.h"

static KCmdLineOptions options[] =
{
	{ "menu <argument>", I18N_NOOP("Menu file"), "systempreferences" },
	{ "e", 0, 0 },
	{ "noembed", I18N_NOOP("Embed Windows"), 0 },
	KCmdLineLastOption
};

int main( int argc, char *argv[] )
{
	// About data
  KAboutData aboutData("systempreferences", I18N_NOOP("System Preferences"),
	  SYSTEM_PREFERENCES_VERSION, I18N_NOOP("System Preferences"),
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

