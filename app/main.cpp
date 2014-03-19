/**
 * Copyright (C) 2009 Ben Cooksley <bcooksley@kde.org>
 *
 * This file was sourced from the System Settings package
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

#include <QCommandLineParser>
#include <KAboutData>

#include "SystemSettingsApp.h"
#include "SettingsBase.h"
#include <QDebug>

int main( int argc, char *argv[] )
{
    SystemSettingsApp application(argc, argv);

    // About data
    KAboutData aboutData("systemsettings", 0, i18n("System Settings"), SYSTEMSETTINGS_STRING_VERSION, i18n("Central configuration center for KDE."), KAboutData::License_GPL, i18n("(c) 2009, Ben Cooksley"));
    aboutData.addAuthor(i18n("Ben Cooksley"), i18n("Maintainer"), "bcooksley@kde.org");
    aboutData.addAuthor(i18n("Mathias Soeken"), i18n("Developer"), "msoeken@informatik.uni-bremen.de");
    aboutData.addAuthor(i18n("Will Stephenson"), i18n("Internal module representation, internal module model"), "wstephenson@kde.org");
    KAboutData::setApplicationData(aboutData);

    aboutData.setProgramIconName("preferences-system");
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    aboutData.setupCommandLine(&parser);
    parser.process(application);
    aboutData.processCommandLine(&parser);

    SettingsBase *mainWindow = new SettingsBase();
    mainWindow->show();
    application.setMainWindow(mainWindow);
    return application.exec();
}
