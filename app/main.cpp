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

#include <iostream>
#include <kworkspace.h>
#include <kdbusservice.h>
#include <KQuickAddons/QtQuickSettings>
#include <KServiceTypeTrader>
#include <KAuthorized>

#include "SystemSettingsApp.h"
#include "SettingsBase.h"

KService::List m_modules;

static bool caseInsensitiveLessThan(const KService::Ptr s1, const KService::Ptr s2)
{
    const int compare = QString::compare(s1->desktopEntryName(),
                                         s2->desktopEntryName(),
                                         Qt::CaseInsensitive);
    return (compare < 0);
}

static void listModules()
{
    // First condition is what systemsettings does, second what kinfocenter does, make sure this is kept in sync
    // We need the exist calls because otherwise the trader language aborts if the property doesn't exist and the second part of the or is not evaluated
    const KService::List services = KServiceTypeTrader::self()->query( QStringLiteral("KCModule"), QStringLiteral("(exist [X-KDE-System-Settings-Parent-Category] and [X-KDE-System-Settings-Parent-Category] != '') or (exist [X-KDE-ParentApp] and [X-KDE-ParentApp] == 'kinfocenter')") );
    for( KService::List::const_iterator it = services.constBegin(); it != services.constEnd(); ++it) {
        const KService::Ptr s = (*it);
        if (!KAuthorized::authorizeControlModule(s->menuId()))
            continue;
        m_modules.append(s);
    }

    std::stable_sort(m_modules.begin(), m_modules.end(), caseInsensitiveLessThan);
}

int main( int argc, char *argv[] )
{
    // Make sure the binary name is either kinfocenter or systemsettings,
    // Anything else will just be considered as "systemsettings"
    const QString executableName = QString::fromUtf8(argv[0]);
    QString binaryName = QStringLiteral("systemsettings");
    BaseMode::ApplicationMode mode = BaseMode::SystemSettings;
    if (executableName.endsWith(QLatin1String("kinfocenter"))) {
        binaryName = QStringLiteral("kinfocenter");
        mode = BaseMode::InfoCenter;
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

    parser.addOption(QCommandLineOption(QStringLiteral("list"), i18n("List all possible modules")));
    parser.addPositionalArgument(QStringLiteral("module"), i18n("Configuration module to open"));
    parser.addOption(QCommandLineOption(QStringLiteral("args"), i18n("Arguments for the module"), QLatin1String("arguments")));

    aboutData.setupCommandLine(&parser);
    parser.process(application);
    aboutData.processCommandLine(&parser);

    if (parser.isSet(QStringLiteral("list"))) {
        std::cout << i18n("The following modules are available:").toLocal8Bit().data() << std::endl;

        listModules();

        int maxLen=0;

        for (KService::List::ConstIterator it = m_modules.constBegin(); it != m_modules.constEnd(); ++it) {
            int len = (*it)->desktopEntryName().length();
            if (len > maxLen)
                maxLen = len;
        }

        for (KService::List::ConstIterator it = m_modules.constBegin(); it != m_modules.constEnd(); ++it) {
            QString entry(QStringLiteral("%1 - %2"));

            entry = entry.arg((*it)->desktopEntryName().leftJustified(maxLen, QLatin1Char(' ')))
                    .arg(!(*it)->comment().isEmpty() ? (*it)->comment()
                                                     : i18n("No description available"));

            std::cout << entry.toLocal8Bit().data() << std::endl;
        }
        return 0;
    }

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
    application.setMainWindow(mainWindow);

    if (parser.positionalArguments().count() == 1) {
        QStringList moduleArgs;
        const QString x = parser.value(QStringLiteral("args"));
        moduleArgs << x.split(QRegExp(QStringLiteral(" +")));

        mainWindow->setStartupModule(parser.positionalArguments().first());
        mainWindow->setStartupModuleArgs(moduleArgs);
    }

    return application.exec();
}
