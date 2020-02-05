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
#include <KCrash>

#include <kworkspace.h>
#include <KQuickAddons/QtQuickSettings>

#include "SystemSettingsApp.h"
#include "SettingsBase.h"

int main( int argc, char *argv[] )
{
    //exec is systemsettings5, but we need the QPT to use the right config from the qApp constructor
    //which is before KAboutData::setApplicationData
    QCoreApplication::setApplicationName(QStringLiteral("systemsettings"));

    KWorkSpace::detectPlatform(argc, argv);
    SystemSettingsApp application(argc, argv);
    KQuickAddons::QtQuickSettings::init();
    KCrash::initialize();

    KLocalizedString::setApplicationDomain("systemsettings");

    // About data
    KAboutData aboutData(QStringLiteral("systemsettings"), i18n("System Settings"), QLatin1String(PROJECT_VERSION), i18n("Central configuration center by KDE."), KAboutLicense::GPL, i18n("(c) 2009, Ben Cooksley"));
    aboutData.addAuthor(i18n("Ben Cooksley"), i18n("Maintainer"), QStringLiteral("bcooksley@kde.org"));
    aboutData.addAuthor(i18n("Mathias Soeken"), i18n("Developer"), QStringLiteral("msoeken@informatik.uni-bremen.de"));
    aboutData.addAuthor(i18n("Will Stephenson"), i18n("Internal module representation, internal module model"), QStringLiteral("wstephenson@kde.org"));

    if (qEnvironmentVariableIsSet("KDE_FULL_SESSION")) {
        aboutData.setDesktopFileName(QStringLiteral("systemsettings"));
    } else {
        aboutData.setDesktopFileName(QStringLiteral("kdesystemsettings"));
    }

    KAboutData::setApplicationData(aboutData);

    application.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    application.setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system")));
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(application);
    aboutData.processCommandLine(&parser);

    SettingsBase *mainWindow = new SettingsBase();
    application.setMainWindow(mainWindow);
    return application.exec();
}
