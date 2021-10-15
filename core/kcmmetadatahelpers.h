/*
 *   SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
 *   SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KAuthorized>
#include <KFileUtils>
#include <KPluginMetaData>
#include <KServiceTypeTrader>
#include <QStandardPaths>
#include <kservice.h>

enum MetaDataSource {
    SystemSettings = 1,
    KInfoCenter = 2,
    All = SystemSettings | KInfoCenter,
};

inline QList<KPluginMetaData> findExternalKCMModules(MetaDataSource source)
{
    const auto findExternalModulesInFilesystem = [](const QString &sourceNamespace, const QString &serviceType) {
        const QString sourceNamespaceDirName = QStringLiteral("plasma/%1/externalmodules").arg(sourceNamespace);
        const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, sourceNamespaceDirName, QStandardPaths::LocateDirectory);
        const QStringList files = KFileUtils::findAllUniqueFiles(dirs, QStringList{QStringLiteral("*.desktop")});

        QList<KPluginMetaData> metaDataList;
        for (const QString &file : files) {
            metaDataList << KPluginMetaData::fromDesktopFile(file, QStringList(serviceType));
        }
        return metaDataList;
    };

    QList<KPluginMetaData> metaDataList;
    if (source & SystemSettings) {
        const auto servicesList = KServiceTypeTrader::self()->query(QStringLiteral("SystemSettingsExternalApp"));
        for (const auto &s : servicesList) {
            const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("kservices5/") + s->entryPath());
            metaDataList << KPluginMetaData::fromDesktopFile(path);
        }
        metaDataList << findExternalModulesInFilesystem(QStringLiteral("systemsettings"), QStringLiteral("systemsettingsexternalapp.desktop"));
    }

    if (source & KInfoCenter) {
        metaDataList << findExternalModulesInFilesystem(QStringLiteral("kinfocenter"), QStringLiteral("infocenterexternalapp.desktop"));
    }

    return metaDataList;
}

inline QList<KPluginMetaData> findKCMsMetaData(MetaDataSource source)
{
    QList<KPluginMetaData> modules;
    QSet<QString> uniquePluginIds;
    // We need the exist calls because otherwise the trader language aborts if the property doesn't exist and the second part of the or is not evaluated
    KService::List services;
    QVector<KPluginMetaData> metaDataList = KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms"));
    if (source & SystemSettings) {
        metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/systemsettings"));
        metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/systemsettings_qwidgets"));
        services += KServiceTypeTrader::self()->query(QStringLiteral("KCModule"), QStringLiteral("[X-KDE-System-Settings-Parent-Category] != ''"));
    }
    if (source & KInfoCenter) {
        metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/kinfocenter"));
        services += KServiceTypeTrader::self()->query(QStringLiteral("KCModule"), QStringLiteral("[X-KDE-ParentApp] == 'kinfocenter'"));
    }
    for (const auto &m : qAsConst(metaDataList)) {
        modules << m;
        auto insertionIterator = uniquePluginIds.insert(m.pluginId());
        Q_ASSERT_X(insertionIterator != uniquePluginIds.end(),
                   Q_FUNC_INFO,
                   qPrintable(QStringLiteral("the plugin %1 was found in multiple namespaces").arg(m.pluginId())));
    }

    for (const auto &s : qAsConst(services)) {
        if (!s->noDisplay() && !uniquePluginIds.contains(s->library()) && KAuthorized::authorizeControlModule(s->menuId())) {
            QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("kservices5/") + s->entryPath());
            modules << KPluginMetaData::fromDesktopFile(path);
            uniquePluginIds << s->library();
        }
    }
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
