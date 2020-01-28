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
#include <kdbusservice.h>
#include <KQuickAddons/QtQuickSettings>

#include "SystemSettingsApp.h"
#include "SettingsBase.h"

int main( int argc, char *argv[] )
{
    // Make sure the binary name is either kinfocenter or systemsettings,
    // Anything else will just be considered as "systemsettings"
    QString binaryName = QString::fromUtf8(argv[0]);
    BaseMode::ApplicationMode mode = BaseMode::InfoCenter;
    if (binaryName != QStringLiteral("kinfocenter")) {
        binaryName = QStringLiteral("systemsettings");
        mode = BaseMode::SystemSettings;
    }

    //exec is systemsettings5, but we need the QPT to use the right config from the qApp constructor
    //which is before KAboutData::setApplicationData
    QCoreApplication::setApplicationName(binaryName);

    KWorkSpace::detectPlatform(argc, argv);
    SystemSettingsApp application(argc, argv);
    KQuickAddons::QtQuickSettings::init();
    KCrash::initialize();

    KLocalizedString::setApplicationDomain(binaryName.toUtf8().constData());

    KAboutData aboutData;

    if (mode == BaseMode::InfoCenter) {
        // About data
        aboutData = KAboutData(QStringLiteral("kinfocenter"), i18n("Info Center"), QLatin1String(PROJECT_VERSION), i18n("Centralized and convenient overview of system information."), KAboutLicense::GPL, i18n("(c) 2009, Ben Cooksley"));
        aboutData.addAuthor(i18n("Ben Cooksley"), i18n("Maintainer"), QStringLiteral("bcooksley@kde.org"));
        aboutData.addAuthor(i18n("Mathias Soeken"), i18n("Developer"), QStringLiteral("msoeken@informatik.uni-bremen.de"));
        aboutData.addAuthor(i18n("Will Stephenson"), i18n("Internal module representation, internal module model"), QStringLiteral("wstephenson@kde.org"));

    } else {
        aboutData = KAboutData(QStringLiteral("systemsettings"), i18n("System Settings"), QLatin1String(PROJECT_VERSION), i18n("Central configuration center by KDE."), KAboutLicense::GPL, i18n("(c) 2009, Ben Cooksley"));
        aboutData.addAuthor(i18n("Ben Cooksley"), i18n("Maintainer"), QStringLiteral("bcooksley@kde.org"));
        aboutData.addAuthor(i18n("Mathias Soeken"), i18n("Developer"), QStringLiteral("msoeken@informatik.uni-bremen.de"));
        aboutData.addAuthor(i18n("Will Stephenson"), i18n("Internal module representation, internal module model"), QStringLiteral("wstephenson@kde.org"));
    }

    application.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    QCommandLineParser parser;

    aboutData.setupCommandLine(&parser);
    parser.process(application);
    aboutData.processCommandLine(&parser);

    if (mode == BaseMode::InfoCenter) {
        aboutData.setDesktopFileName(QStringLiteral("org.kde.kinfocenter"));
        application.setWindowIcon(QIcon::fromTheme(QStringLiteral("hwinfo")));

    } else {
        application.setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system")));

        if (qEnvironmentVariableIsSet("KDE_FULL_SESSION")) {
            aboutData.setDesktopFileName(QStringLiteral("systemsettings"));
        } else {
            aboutData.setDesktopFileName(QStringLiteral("kdesystemsettings"));
        }
    }

    KAboutData::setApplicationData(aboutData);

   
    SettingsBase *mainWindow = new SettingsBase(mode);
    mainWindow->show();
    application.setMainWindow(mainWindow);
    return application.exec();
}
