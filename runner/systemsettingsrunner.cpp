/*
    SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2016-2020 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "systemsettingsrunner.h"

#include <algorithm>

#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QMimeData>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QUrl>
#include <QUrlQuery>

#include <KActivities/ResourceInstance>
#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KNotificationJobUiDelegate>
#include <KSycoca>

#include "../core/kcmmetadatahelpers.h"

K_PLUGIN_CLASS_WITH_JSON(SystemsettingsRunner, "systemsettingsrunner.json")

SystemsettingsRunner::SystemsettingsRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : Plasma::AbstractRunner(parent, metaData, args)
{
    setObjectName(QStringLiteral("SystemsettingsRunner"));
    setPriority(AbstractRunner::HighestPriority);

    addSyntax(Plasma::RunnerSyntax(QStringLiteral(":q:"), i18n("Finds system settings modules whose names or descriptions match :q:")));
    // teardown is called in the main thread when all matches are over
    connect(this, &SystemsettingsRunner::teardown, this, [this]() {
        m_modules.clear();
    });
}

void SystemsettingsRunner::match(Plasma::RunnerContext &context)
{
    {
        // The match method is called multithreaded, to make sure we do not start multiple plugin searches or
        // write to the list in different threads the lock is used
        QMutexLocker lock(&m_mutex);
        if (m_modules.isEmpty()) {
            KSycoca::disableAutoRebuild();
            m_modules = findKCMsMetaData(MetaDataSource::All);
        }
    }
    matchNameKeywordAndGenericName(context);
}

void SystemsettingsRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    const auto data = match.data().value<KPluginMetaData>();

    KIO::CommandLauncherJob *job = nullptr;
    if (isKinfoCenterKcm(data)) {
        job = new KIO::CommandLauncherJob(QStringLiteral("kinfocenter"), {data.pluginId()});
        job->setDesktopName(QStringLiteral("org.kde.kinfocenter"));
    } else if (!data.value(QStringLiteral("X-KDE-System-Settings-Parent-Category")).isEmpty()) {
        job = new KIO::CommandLauncherJob(QStringLiteral("systemsettings5"), {data.pluginId()});
        job->setDesktopName(QStringLiteral("org.kde.systemsettings"));
    } else {
        // If we have created the KPluginMetaData from a desktop file KCMShell needs the pluginId, otherwise we can give
        // it the absolute path to the plugin. That works in any case, see commit 866d730fd098775f6b16cc8ba15974af80700d12 in kde-cli-tools
        bool isDesktopFile = data.metaDataFileName().endsWith(QLatin1String(".desktop"));
        job = new KIO::CommandLauncherJob(QStringLiteral("kcmshell5"), {isDesktopFile ? data.pluginId() : data.fileName()});
    }
    auto *delegate = new KNotificationJobUiDelegate;
    delegate->setAutoErrorHandlingEnabled(true);
    job->setUiDelegate(delegate);
    job->start();

    KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("systemsettings:") + data.pluginId()), QStringLiteral("org.kde.krunner"));
}

QMimeData *SystemsettingsRunner::mimeDataForMatch(const Plasma::QueryMatch &match)
{
    const auto value = match.data().value<KPluginMetaData>();
    if (value.isValid() && value.metaDataFileName().endsWith(QLatin1String(".desktop"))) {
        auto *data = new QMimeData();
        data->setUrls(QList<QUrl>{QUrl::fromLocalFile(value.metaDataFileName())});
        return data;
    }
    return nullptr;
}

void SystemsettingsRunner::setupMatch(const KPluginMetaData &data, Plasma::QueryMatch &match)
{
    const QString name = data.name();

    match.setText(name);
    const QString genericName = data.value(QStringLiteral("GenericName"));
    if (!genericName.isEmpty() && genericName != name) {
        match.setSubtext(genericName);
    } else if (!data.description().isEmpty()) {
        match.setSubtext(data.description());
    }

    if (!data.iconName().isEmpty()) {
        match.setIconName(data.iconName());
    }
    match.setData(QVariant::fromValue(data));
}

void SystemsettingsRunner::matchNameKeywordAndGenericName(Plasma::RunnerContext &ctx)
{
    QList<Plasma::QueryMatch> matches;
    // Splitting the query term to match using subsequences
    const QStringList queryList = ctx.query().split(QLatin1Char(' '));

    for (const KPluginMetaData &data : qAsConst(m_modules)) {
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::CompletionMatch);
        setupMatch(data, match);
        qreal relevance = -1;

        auto checkMatchAndRelevance = [queryList, data, &relevance](const QString &value, qreal relevanceValue) {
            if (value.contains(queryList.first(), Qt::CaseInsensitive)) {
                relevance = relevanceValue;
                return true;
            }
            for (const QString &query : queryList) {
                if (relevance == -1 && value.contains(query, Qt::CaseInsensitive)) {
                    relevance = 0.5;
                    return true;
                }
            }
            return false;
        };

        if (checkMatchAndRelevance(data.name(), 0.8)) {
            if (data.name().startsWith(queryList[0], Qt::CaseInsensitive)) {
                relevance += 0.1;
            }
        } else {
            if (!checkMatchAndRelevance(data.value(QStringLiteral("GenericName")), 0.65) && !checkMatchAndRelevance(data.description(), 0.5)) {
                continue;
            }
        }

        if (isKinfoCenterKcm(data)) {
            match.setMatchCategory(i18n("System Information"));
        } else {
            match.setMatchCategory(i18n("System Settings"));
        }
        // KCMs are, on the balance, less relevant. Drop it ever so much. So they may get outscored
        // by an otherwise equally applicable match.
        relevance -= .001;

        match.setRelevance(relevance);

        matches << match;
    }
    ctx.addMatches(matches);
}

#include "systemsettingsrunner.moc"
