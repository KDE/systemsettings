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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KAboutData>
#include <KCmdLineArgs>
#include <KIcon>
#include <KLocale>
#include <KUniqueApplication>

#include "mainwindow.h"
#include "version.h"

#include <iostream>

int main( int argc, char *argv[] )
{
	KLocale::setMainCatalog("systemsettings");
	// About data
	KAboutData aboutData("systemsettings", 0, ki18n("System Settings"),
			SYSTEM_SETTINGS_VERSION, ki18n("System Settings"),
			KAboutData::License_LGPL, ki18n("(c) 2005, Benjamin C. Meyer; (c) 2007, Canonical Ltd; (c) 2007 Will Stephenson"));
	aboutData.addAuthor(ki18n("Benjamin C. Meyer"), ki18n("Author"),
			"ben+systempreferences@meyerhome.net");
	aboutData.addAuthor(ki18n("Jonathan Riddell"), ki18n("Contributor"),
			"jriddell@ubuntu.com");
	aboutData.addAuthor(ki18n("Michael D. Stemle"), ki18n("Contributor"),
			"manchicken@notsosoft.net");
	aboutData.addAuthor(ki18n("Simon Edwards"), ki18n("Contributor"),
			"simon@simonzone.com");
	aboutData.addAuthor(ki18n("Will Stephenson"), ki18n("Contributor"),
			"wstephenson@kde.org");
	aboutData.addAuthor(ki18n("Ellen Reitmayr"), ki18n("Usability"),
			"ellen@kde.org");
	KCmdLineArgs::init(argc, argv, &aboutData);

	if (!KUniqueApplication::start()) {
		std::cerr << "This program is already running." << std::endl;
		return 0;
	}
	KUniqueApplication application;

	QApplication::setWindowIcon(KIcon("preferences-system"));

	MainWindow *mainWindow = new MainWindow();

	mainWindow->show();

	return application.exec();
}
