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
            m_modules = findKCMsMetaData(MetaDataSource::All, false);
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
        job = new KIO::CommandLauncherJob(QStringLiteral("systemsettings"), {data.pluginId()});
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
    if (value.isValid()) {
        if (value.metaDataFileName().endsWith(QLatin1String(".desktop"))) {
            auto *data = new QMimeData();
            data->setUrls(QList<QUrl>{QUrl::fromLocalFile(value.metaDataFileName())});
            return data;
        }
        if (KService::Ptr ptr = KService::serviceByStorageId(value.pluginId() + QLatin1String(".desktop"))) {
            auto *data = new QMimeData();
            data->setUrls(QList<QUrl>{QUrl::fromLocalFile(ptr->entryPath())});
            return data;
        }
    }
    return nullptr;
}

void SystemsettingsRunner::setupMatch(const KPluginMetaData &data, Plasma::QueryMatch &match)
{
    const QString name = data.name();

    match.setText(name);
    if (data.metaDataFileName().endsWith(QLatin1String(".desktop"))) {
        QUrl url(data.metaDataFileName());
        url.setScheme(QStringLiteral("applications"));
        match.setUrls({url});
    } else {
        QUrl url(data.pluginId());
        url.setScheme(QStringLiteral("applications"));
        match.setUrls({url});
    }
    const QString genericName = data.value(QStringLiteral("GenericName"));
    if (!genericName.isEmpty() && genericName != name) {
        match.setSubtext(genericName);
    } else if (!data.description().isEmpty()) {
        match.setSubtext(data.description());
    }

    if (!data.iconName().isEmpty()) {
        match.setIconName(data.iconName());
    }
    match.setId(data.pluginId()); // KRunner needs the id to adjust the relevance for often launched KCMs
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
            // check if the generic name or description matches
            if (!checkMatchAndRelevance(data.value(QStringLiteral("GenericName")), 0.65) && !checkMatchAndRelevance(data.description(), 0.5)) {
                // if not, check the keyowords
                const QString &query = ctx.query();
                const QStringList keywords = data.value(QStringLiteral("X-KDE-Keywords")).split(QLatin1Char(','));
                bool anyKeywordMatches = std::any_of(keywords.begin(), keywords.end(), [&query](const QString &keyword) {
                    return keyword.startsWith(query, Qt::CaseInsensitive);
                });
                if (anyKeywordMatches && keywords.contains(query, Qt::CaseInsensitive)) {
                    relevance = 0.5; // If the keyword matches exactly we give it the same relevance as if the description matched
                } else if (anyKeywordMatches) {
                    relevance = 0.2; // give it a lower relevance than if it had been found by name or description
                } else {
                    continue; // we haven't found any matching keyword, skip this KCM
                }
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
