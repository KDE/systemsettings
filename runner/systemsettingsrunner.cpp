/*
    SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2016-2020 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "systemsettingsrunner.h"

#include <algorithm>

#include <QMimeData>

#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QStandardPaths>
#include <QUrl>
#include <QUrlQuery>

#include <KActivities/ResourceInstance>
#include <KLocalizedString>
#include <KNotificationJobUiDelegate>
#include <KServiceAction>
#include <KServiceTypeTrader>
#include <KStringHandler>
#include <KSycoca>

#include <KIO/ApplicationLauncherJob>
#include <KIO/DesktopExecParser>

K_PLUGIN_CLASS_WITH_JSON(SystemsettingsRunner, "systemsettingsrunner.json")

/**
 * @brief Finds all KServices for a given runner query
 */
class SystemsettingsFinder
{
public:
    SystemsettingsFinder(SystemsettingsRunner *runner)
        : m_runner(runner)
    {
    }

    void match(Plasma::RunnerContext &context)
    {
        if (!context.isValid()) {
            return;
        }

        KSycoca::disableAutoRebuild();

        term = context.query();

        matchNameKeywordAndGenericName();

        context.addMatches(matches);
    }

private:
    qreal increaseMatchRelavance(const KService::Ptr &service, const QVector<QStringRef> &strList, const QString &category)
    {
        // Increment the relevance based on all the words (other than the first) of the query list
        qreal relevanceIncrement = 0;

        for (int i = 1; i < strList.size(); ++i) {
            const auto &str = strList.at(i);
            if (category == QLatin1String("Name")) {
                if (service->name().contains(str, Qt::CaseInsensitive)) {
                    relevanceIncrement += 0.01;
                }
            } else if (category == QLatin1String("GenericName")) {
                if (service->genericName().contains(str, Qt::CaseInsensitive)) {
                    relevanceIncrement += 0.01;
                }
            } else if (category == QLatin1String("Exec")) {
                if (service->exec().contains(str, Qt::CaseInsensitive)) {
                    relevanceIncrement += 0.01;
                }
            } else if (category == QLatin1String("Comment")) {
                if (service->comment().contains(str, Qt::CaseInsensitive)) {
                    relevanceIncrement += 0.01;
                }
            }
        }

        return relevanceIncrement;
    }

    QString generateQuery(const QVector<QStringRef> &strList)
    {
        QString keywordTemplate = QStringLiteral("exist Keywords");
        QString genericNameTemplate = QStringLiteral("exist GenericName");
        QString nameTemplate = QStringLiteral("exist Name");
        QString commentTemplate = QStringLiteral("exist Comment");

        // Search for applications which are executable and the term case-insensitive matches any of
        // * a substring of one of the keywords
        // * a substring of the GenericName field
        // * a substring of the Name field
        // Note that before asking for the content of e.g. Keywords and GenericName we need to ask if
        // they exist to prevent a tree evaluation error if they are not defined.
        for (const QStringRef &str : strList) {
            keywordTemplate += QStringLiteral(" and '%1' ~subin Keywords").arg(str.toString());
            genericNameTemplate += QStringLiteral(" and '%1' ~~ GenericName").arg(str.toString());
            nameTemplate += QStringLiteral(" and '%1' ~~ Name").arg(str.toString());
            commentTemplate += QStringLiteral(" and '%1' ~~ Comment").arg(str.toString());
        }

        QString finalQuery = QStringLiteral("exist Exec and ( (%1) or (%2) or (%3) or ('%4' ~~ Exec) or (%5) )")
                                 .arg(keywordTemplate, genericNameTemplate, nameTemplate, strList[0].toString(), commentTemplate);

        return finalQuery;
    }

    void setupMatch(const KService::Ptr &service, Plasma::QueryMatch &match)
    {
        const QString name = service->name();

        match.setText(name);

        QUrl url(service->storageId());
        url.setScheme(QStringLiteral("applications"));
        match.setData(url);
        QString exec = service->exec();
        const QStringList resultingArgs = KIO::DesktopExecParser(KService(QString(), exec, QString()), {}).resultingArguments();
        match.setId(QStringLiteral("exec://") + resultingArgs.join(QLatin1Char(' ')));
        if (!service->genericName().isEmpty() && service->genericName() != name) {
            match.setSubtext(service->genericName());
        } else if (!service->comment().isEmpty()) {
            match.setSubtext(service->comment());
        }

        if (!service->icon().isEmpty()) {
            match.setIconName(service->icon());
        }
    }

    void matchNameKeywordAndGenericName()
    {
        // Splitting the query term to match using subsequences
        QVector<QStringRef> queryList = term.splitRef(QLatin1Char(' '));

        // If the term length is < 3, no real point searching the Keywords and GenericName
        const QString query = generateQuery(queryList);

        const KService::List services = KServiceTypeTrader::self()->query(QStringLiteral("KCModule"), query);

        for (const KService::Ptr &service : qAsConst(services)) {
            if (service->noDisplay()) {
                continue;
            }

            Plasma::QueryMatch match(m_runner);
            match.setType(Plasma::QueryMatch::CompletionMatch);
            setupMatch(service, match);
            qreal relevance(0.6);

            if (service->name().contains(queryList[0], Qt::CaseInsensitive)) {
                relevance = 0.8;
                relevance += increaseMatchRelavance(service, queryList, QStringLiteral("Name"));

                if (service->name().startsWith(queryList[0], Qt::CaseInsensitive)) {
                    relevance += 0.1;
                }
            } else if (service->genericName().contains(queryList[0], Qt::CaseInsensitive)) {
                relevance = 0.65;
                relevance += increaseMatchRelavance(service, queryList, QStringLiteral("GenericName"));

                if (service->genericName().startsWith(queryList[0], Qt::CaseInsensitive)) {
                    relevance += 0.05;
                }
            } else if (service->comment().contains(queryList[0], Qt::CaseInsensitive)) {
                relevance = 0.5;
                relevance += increaseMatchRelavance(service, queryList, QStringLiteral("Comment"));

                if (service->comment().startsWith(queryList[0], Qt::CaseInsensitive)) {
                    relevance += 0.05;
                }
            }

            if (service->parentApp() == QStringLiteral("kinfocenter")) {
                match.setMatchCategory(i18n("System Information"));
            } else {
                match.setMatchCategory(i18n("System Settings"));
            }
            // KCMs are, on the balance, less relevant. Drop it ever so much. So they may get outscored
            // by an otherwise equally applicable match.
            relevance -= .001;

            // qCDebug(RUNNER_SERVICES) << service->name() << "is this relevant:" << relevance;
            match.setRelevance(relevance);

            matches << match;
        }
    }

    SystemsettingsRunner *m_runner;

    QList<Plasma::QueryMatch> matches;
    QString term;
};

SystemsettingsRunner::SystemsettingsRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : Plasma::AbstractRunner(parent, metaData, args)
{
    setObjectName(QStringLiteral("SystemsettingsRunner"));
    setPriority(AbstractRunner::HighestPriority);

    addSyntax(Plasma::RunnerSyntax(QStringLiteral(":q:"), i18n("Finds system settings modules whose names or descriptions match :q:")));
}

SystemsettingsRunner::~SystemsettingsRunner() = default;

void SystemsettingsRunner::match(Plasma::RunnerContext &context)
{
    // This helper class aids in keeping state across numerous
    // different queries that together form the matches set.
    SystemsettingsFinder finder(this);
    finder.match(context);
}

void SystemsettingsRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    const QUrl dataUrl = match.data().toUrl();

    KService::Ptr service = KService::serviceByStorageId(dataUrl.path());
    if (!service) {
        return;
    }

    KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("applications:") + service->storageId()), QStringLiteral("org.kde.krunner"));

    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(service);
    auto *delegate = new KNotificationJobUiDelegate;
    delegate->setAutoErrorHandlingEnabled(true);
    job->setUiDelegate(delegate);
    job->start();
}

QMimeData *SystemsettingsRunner::mimeDataForMatch(const Plasma::QueryMatch &match)
{
    const QUrl dataUrl = match.data().toUrl();

    KService::Ptr service = KService::serviceByStorageId(dataUrl.path());
    if (!service) {
        return nullptr;
    }

    QString path = service->entryPath();
    if (!QDir::isAbsolutePath(path)) {
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kservices5/") + path);
    }

    if (path.isEmpty()) {
        return nullptr;
    }

    auto *data = new QMimeData();
    data->setUrls(QList<QUrl>{QUrl::fromLocalFile(path)});
    return data;
}

#include "systemsettingsrunner.moc"
