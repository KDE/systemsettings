/*
 *   SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KAuthorized>
#include <KPluginMetaData>
#include <KServiceTypeTrader>
#include <QStandardPaths>
#include <kservice.h>

enum MetaDataSource {
    SystemSettings = 1,
    KInfoCenter = 2,
    All = SystemSettings | KInfoCenter,
};
inline QList<KPluginMetaData> findKCMsMetaData(MetaDataSource source)
{
    QList<KPluginMetaData> modules;
    QSet<QString> uniquePluginIds;
    // We need the exist calls because otherwise the trader language aborts if the property doesn't exist and the second part of the or is not evaluated
    KService::List services;
    QVector<KPluginMetaData> metaDataList = KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms"));
    if (source & SystemSettings) {
        metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/systemsettings"));
        services += KServiceTypeTrader::self()->query(QStringLiteral("KCModule"), QStringLiteral("[X-KDE-System-Settings-Parent-Category] != ''"));
        services += KServiceTypeTrader::self()->query(QStringLiteral("SystemSettingsExternalApp"));
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
                   qPrintable(QStringLiteral("the plugin %1 was found in mutiple namespaces").arg(m.pluginId())));
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
