/*
 *   SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
 *   SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KAuthorized>
#include <KCModuleData>
#include <KFileUtils>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <QGuiApplication>
#include <QStandardPaths>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <KServiceTypeTrader>
#endif
#include <kservice.h>
#include <set>

#include "../systemsettings_app_debug.h"

enum MetaDataSource {
    SystemSettings = 1,
    KInfoCenter = 2,
    All = SystemSettings | KInfoCenter,
};

inline QList<KPluginMetaData> findExternalKCMModules(MetaDataSource source)
{
    const auto findExternalModulesInFilesystem = [](const QString &sourceNamespace) {
        const QString sourceNamespaceDirName = QStringLiteral("plasma/%1/externalmodules").arg(sourceNamespace);
        const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, sourceNamespaceDirName, QStandardPaths::LocateDirectory);
        const QStringList files = KFileUtils::findAllUniqueFiles(dirs, QStringList{QStringLiteral("*.desktop")});

        QList<KPluginMetaData> metaDataList;
        for (const QString &file : files) {
            KService service(file);
            QJsonObject kplugin;
            kplugin.insert(QLatin1String("Name"), service.name());
            kplugin.insert(QLatin1String("Icon"), service.icon());
            kplugin.insert(QLatin1String("Description"), service.comment());

            QJsonObject root;
            root.insert(QLatin1String("KPlugin"), kplugin);
            root.insert(QLatin1String("X-KDE-Weight"), service.property(QStringLiteral("X-KDE-Weight")).toInt());
            root.insert(QLatin1String("X-KDE-KInfoCenter-Category"), service.property(QStringLiteral("X-KDE-KInfoCenter-Category")).toString());
            root.insert(QLatin1String("X-KDE-System-Settings-Category"), service.property(QStringLiteral("X-KDE-System-Settings-Category")).toString());
            root.insert(QLatin1String("IsExternalApp"), true);

            metaDataList << KPluginMetaData(root, file);
        }
        return metaDataList;
    };

    QList<KPluginMetaData> metaDataList;
    if (source & SystemSettings) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const auto servicesList = KServiceTypeTrader::self()->query(QStringLiteral("SystemSettingsExternalApp"));
        for (const auto &s : servicesList) {
            const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("kservices5/") + s->entryPath());
            metaDataList << KPluginMetaData::fromDesktopFile(path);
        }
#endif
        metaDataList << findExternalModulesInFilesystem(QStringLiteral("systemsettings"));
    }

    if (source & KInfoCenter) {
        metaDataList << findExternalModulesInFilesystem(QStringLiteral("kinfocenter"));
    }

    return metaDataList;
}

inline QList<KPluginMetaData> findKCMsMetaData(MetaDataSource source, bool useSystemsettingsConstraint = true)
{
    QList<KPluginMetaData> modules;
    std::set<QString> uniquePluginIds;

    auto filter = [](const KPluginMetaData &data) {
        const auto supportedPlatforms = data.value(QStringLiteral("X-KDE-OnlyShowOnQtPlatforms"), QStringList());
        return supportedPlatforms.isEmpty() || supportedPlatforms.contains(qGuiApp->platformName());
    };

    // We need the exist calls because otherwise the trader language aborts if the property doesn't exist and the second part of the or is not evaluated
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    KService::List services;
#endif
    QVector<KPluginMetaData> metaDataList = KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms"), filter);
    if (source & SystemSettings) {
        metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/systemsettings"), filter);
        metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/systemsettings_qwidgets"), filter);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        services +=
            KServiceTypeTrader::self()->query(QStringLiteral("KCModule"),
                                              useSystemsettingsConstraint ? QStringLiteral("[X-KDE-System-Settings-Parent-Category] != ''") : QString());
#endif
    }
    if (source & KInfoCenter) {
        metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/kinfocenter"), filter);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        services += KServiceTypeTrader::self()->query(QStringLiteral("KCModule"), QStringLiteral("[X-KDE-ParentApp] == 'kinfocenter'"));
#endif
    }
    for (const auto &m : qAsConst(metaDataList)) {
        // We check both since porting a module to loading view KPluginMetaData drops ".desktop" from the pluginId()
        if (!KAuthorized::authorizeControlModule(m.pluginId()) || !KAuthorized::authorizeControlModule(m.pluginId().append(QStringLiteral(".desktop")))) {
            continue;
        }
        modules << m;
        const bool inserted = uniquePluginIds.insert(m.pluginId()).second;
        if (!inserted) {
            qWarning() << "the plugin" << m.pluginId() << " was found in multiple namespaces";
        }
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    for (const auto &s : qAsConst(services)) {
        if (!s->noDisplay() && !s->exec().isEmpty() && KAuthorized::authorizeControlModule(s->menuId())) {
            const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("kservices5/") + s->entryPath());
            const KPluginMetaData data = KPluginMetaData::fromDesktopFile(path);
            const bool inserted = uniquePluginIds.insert(data.pluginId()).second;
            const static QStringList ignoredPlugins = {
                QStringLiteral("kcm_driver_manager"), // archived, but still on KDE Neon
                QStringLiteral("kcm_kwallet5"), // already ported, but part of KDE Gear
                QStringLiteral("kcm_kup"), // already ported, but part of KDE Gear
                QStringLiteral("kcm_ssl"), // frameworks, will be removed in KF6
            };
            if (inserted && !ignoredPlugins.contains(data.pluginId())) {
                qWarning(SYSTEMSETTINGS_APP_LOG)
                    << "Loading KCModule" << data.pluginId()
                    << "using KServicetypeTrader, please install QML KCMs in the plasma/kcms/systemsettings namespace and QWidget KCMs in "
                       "plasma/kcms/systemsettings/qwidgets with embedded json metadata";
                modules << data;
            }
        }
    }
#endif
    std::stable_sort(modules.begin(), modules.end(), [](const KPluginMetaData &m1, const KPluginMetaData &m2) {
        return QString::compare(m1.pluginId(), m2.pluginId(), Qt::CaseInsensitive) < 0;
    });
    return modules;
}

inline bool isKinfoCenterKcm(const KPluginMetaData &data)
{
    // KServiceTypeTrader compat
    if (data.value(QStringLiteral("X-KDE-ParentApp")) == QLatin1String("kinfocenter")) {
        return true;
    }
    // external module or a KCM in the namespace
    if (data.fileName().contains(QLatin1String("/kinfocenter/"))) {
        return true;
    }
    return false;
}

inline KCModuleData *loadModuleData(const KPluginMetaData &data)
{
    if (!data.isValid()) {
        return nullptr;
    }
    KCModuleData *moduleData = nullptr;
    auto loadFromMetaData = [&moduleData](const KPluginMetaData &data) {
        if (data.isValid()) {
            auto factory = KPluginFactory::loadFactory(data).plugin;
            moduleData = factory ? factory->create<KCModuleData>() : nullptr;
        }
    };
    loadFromMetaData(data);
    if (!moduleData) {
        loadFromMetaData(KPluginMetaData(QStringLiteral("kcms/") + data.fileName()));
    }
    return moduleData;
}
