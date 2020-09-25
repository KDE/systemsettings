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
#include <KDBusService>
#include <KQuickAddons/QtQuickSettings>
#include <KServiceTypeTrader>
#include <KAuthorized>
#include <KWindowSystem>

#include "SettingsBase.h"

#include <QQuickWindow>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QProcess>

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


/**
 * The ffsNvidia method provides 2 functions
 * A recent regression means that on Nvidia, suspend, the first attempt to make a context current reports
 * a graphic context loss event.
 * This messes with QQuickWidget which doesn't expect the context to fail on startup.
 *
 * Querying it once (done by makeCurrent) will clear that flag and everything behaves afterwards
 * See KDE bug 424592

 * In addition we can detect the common bug of libGL being broken and restart in software mode
 * This commonly happens with a certain driver updates, but the new lib GL requires running against a specific kernel
 * version that is not yet loaded
 * KDE bug 426019

 * This method will respawn in software mode.
 *
 * @returns true if the GL context is fine. If false the user should exit
*/

static bool firstFrameSaftetyNvidia()
{
        // No other backend is set, so it must be openGL
        if (!QQuickWindow::sceneGraphBackend().isEmpty()) {
            return true;
        }
        // environment variables don't set QQuickWindow's backend till the
        // first window  is created, check explicitly
        if (qEnvironmentVariableIsSet("QT_QUICK_BACKEND")) {
            return true;
        }

        QOpenGLContext context;
        context.create();
        QOffscreenSurface surface;
        surface.create();
        bool success = context.makeCurrent(&surface);
        if (!success) {
            QProcess restartProc;
            QStringList arguments = qApp->arguments();
            Q_ASSERT(arguments.count() > 0);
            restartProc.setProgram(arguments.takeFirst());
            restartProc.setArguments(arguments);
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert(QStringLiteral("QT_QUICK_BACKEND"), QStringLiteral("software"));
            restartProc.setProcessEnvironment(env);
            restartProc.startDetached();
            restartProc.waitForStarted();
            return false;
        }
        return true;
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

    QApplication application(argc, argv);
    application.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    KAboutData aboutData;

    if (mode == BaseMode::InfoCenter) {
        // About data
        aboutData = KAboutData(QStringLiteral("kinfocenter"), i18n("Info Center"), QLatin1String(PROJECT_VERSION), i18n("Centralized and convenient overview of system information."), KAboutLicense::GPL, i18n("(c) 2009, Ben Cooksley"));
        aboutData.setDesktopFileName(QStringLiteral("org.kde.kinfocenter"));

        application.setWindowIcon(QIcon::fromTheme(QStringLiteral("hwinfo")));

    } else {
        aboutData = KAboutData(QStringLiteral("systemsettings"), i18n("System Settings"), QLatin1String(PROJECT_VERSION), i18n("Central configuration center by KDE."), KAboutLicense::GPL, i18n("(c) 2009, Ben Cooksley"));

        if (qEnvironmentVariableIsSet("KDE_FULL_SESSION")) {
            aboutData.setDesktopFileName(QStringLiteral("systemsettings"));
        } else {
            aboutData.setDesktopFileName(QStringLiteral("kdesystemsettings"));
        }

        application.setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system")));
    }

    aboutData.addAuthor(i18n("Ben Cooksley"), i18n("Maintainer"), QStringLiteral("bcooksley@kde.org"));
    aboutData.addAuthor(i18n("Mathias Soeken"), i18n("Developer"), QStringLiteral("msoeken@informatik.uni-bremen.de"));
    aboutData.addAuthor(i18n("Will Stephenson"), i18n("Internal module representation, internal module model"), QStringLiteral("wstephenson@kde.org"));

    KAboutData::setApplicationData(aboutData);

    KQuickAddons::QtQuickSettings::init();

    // If the first frame safety method fails it will auto respawn system settings, we should exit our instance
    if (!firstFrameSaftetyNvidia()) {
        return 0;
    }

    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));

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

    if (parser.positionalArguments().count() > 1) {
        std::cerr << "Only one module argument may be passed" << std::endl;
        return -1;
    }

    const QStringList args = parser.value(QStringLiteral("args")).split(QRegExp(QStringLiteral(" +")), Qt::SkipEmptyParts);
    QString startupModule;

    if (parser.positionalArguments().count() == 1) {
        startupModule = parser.positionalArguments().first();
    }

    if (!args.isEmpty() && startupModule.isEmpty()) {
        std::cerr << "Arguments may only be passed when specifying a module" << std::endl;
        return -1;
    }

    KDBusService service(KDBusService::Unique);

    KWorkSpace::detectPlatform(argc, argv);
    KCrash::initialize();
    KLocalizedString::setApplicationDomain(binaryName.toUtf8().constData());

    SettingsBase *mainWindow = new SettingsBase(mode);

    QObject::connect(&service, &KDBusService::activateRequested, mainWindow, [mainWindow](const QStringList &arguments, const QString &workingDirectory) {
        Q_UNUSED(workingDirectory);

        // We can't use startupModule and args from above since they come from the existing instance, so we need to parse arguments.
        // We don't need to do the error checking again though.
        QCommandLineParser parser;
        parser.addPositionalArgument(QStringLiteral("module"), i18n("Configuration module to open"));
        parser.addOption(QCommandLineOption(QStringLiteral("args"), i18n("Arguments for the module"), QLatin1String("arguments")));

        parser.parse(arguments);

        const QStringList args = parser.value(QStringLiteral("args")).split(QRegExp(QStringLiteral(" +")), Qt::SkipEmptyParts);
        QString startupModule;

        if (parser.positionalArguments().count() == 1) {
            startupModule = parser.positionalArguments().first();
        }

        if (!startupModule.isEmpty()) {
            mainWindow->setStartupModule(startupModule);
            mainWindow->setStartupModuleArgs(args);
            mainWindow->reloadStartupModule();
        }

        KWindowSystem::forceActiveWindow(mainWindow->winId());
    });

    if (!startupModule.isEmpty()) {
        mainWindow->setStartupModule(startupModule);
        mainWindow->setStartupModuleArgs(args);
    }

    return application.exec();
}
