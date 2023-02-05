/*
    SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2016-2020 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2022 Natalie Clarius <natalie_clarius@yahoo.de>

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
    matchNameKeyword(context);
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
        job->setDesktopName(QStringLiteral("systemsettings"));
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
    match.setSubtext(data.description());

    if (!data.iconName().isEmpty()) {
        match.setIconName(data.iconName());
    }
    match.setId(data.pluginId()); // KRunner needs the id to adjust the relevance for often launched KCMs
    match.setData(QVariant::fromValue(data));
}

void SystemsettingsRunner::matchNameKeyword(Plasma::RunnerContext &ctx)
{
    QList<Plasma::QueryMatch> matches;
    const QString query = ctx.query();

    for (const KPluginMetaData &data : qAsConst(m_modules)) {
        const QString name = data.name();
        const QString description = data.description();
        const QStringList keywords = data.value(QStringLiteral("X-KDE-Keywords")).split(QLatin1Char(','));

        Plasma::QueryMatch match(this);
        setupMatch(data, match);
        qreal relevance = -1;
        Plasma::QueryMatch::Type type = Plasma::QueryMatch::CompletionMatch;

        auto checkMatchAndRelevance = [query, data, &relevance](const QString &value, qreal relevanceValue) {
            if (value.startsWith(query, Qt::CaseInsensitive)) {
                relevance = relevanceValue + 0.1;
                return true;
            }
            const QStringList queryWords{query.split(QLatin1Char(' '))};
            for (const QString &queryWord : queryWords) {
                if (relevance == -1 && queryWord.length() > 3 && value.contains(queryWord, Qt::CaseInsensitive)) {
                    relevance = relevanceValue;
                    return true;
                }
            }
            return false;
        };

        // check for matches and set relevance
        if (checkMatchAndRelevance(name, 0.8)) { // name starts with query or contains any query word
        } else if (checkMatchAndRelevance(description, 0.5)) { // description starts with query or contains any query word
        } else if (std::any_of(keywords.begin(), keywords.end(), [&query](const QString &keyword) {
                       return keyword.startsWith(query, Qt::CaseInsensitive);
                   })) {
            if (keywords.contains(query, Qt::CaseInsensitive)) { // any of the keywords matches query
                relevance = 0.5;
            } else { // any of the keywords starts with query
                relevance = 0.2;
            }
        } else { // none of the properties matches
            continue; // skip this KCM
        }

        // set type
        if (name.compare(query, Qt::CaseInsensitive) == 0) { // name matches exactly
            type = Plasma::QueryMatch::ExactMatch;
        } else if (name.startsWith(query, Qt::CaseInsensitive) || description.startsWith(query, Qt::CaseInsensitive)) { // name or description matches as start
            type = Plasma::QueryMatch::PossibleMatch;
        } else if (keywords.contains(query, Qt::CaseInsensitive)) { // any of the keywords matches exactly
            type = Plasma::QueryMatch::PossibleMatch;
        }

        match.setRelevance(relevance);
        match.setType(type);

        if (isKinfoCenterKcm(data)) {
            match.setMatchCategory(i18n("System Information"));
        } else {
            match.setMatchCategory(i18n("System Settings"));
        }

        matches << match;
    }
    ctx.addMatches(matches);
}

#include "systemsettingsrunner.moc"
