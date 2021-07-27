/*
    SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef SYSTEMSETTINGSRUNNER_H
#define SYSTEMSETTINGSRUNNER_H

#include <KService>

#include <KRunner/AbstractRunner>

class SystemsettingsRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    SystemsettingsRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~SystemsettingsRunner() override;

    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

protected Q_SLOTS:
    QMimeData *mimeDataForMatch(const Plasma::QueryMatch &match) override;

protected:
    void setupMatch(const KService::Ptr &service, Plasma::QueryMatch &action);
};

#endif
