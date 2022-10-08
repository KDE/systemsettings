/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ExternalAppModule.h"

#include <KCModuleInfo>
#include <KIO/ApplicationLauncherJob>
#include <KIO/JobUiDelegateFactory>

ExternalAppModule::ExternalAppModule(QWidget *parent, const KService::Ptr &module)
    : module(module)
{
    Q_UNUSED(parent)

    firstShow = true;
    externalModule.setupUi(this);
    QString moduleName = module->name();
    if (moduleName.isEmpty()) {
        moduleName = module->property(QStringLiteral("X-KDE-PluginInfo-Name"), QVariant::String).toString();
        if (!moduleName.isEmpty()) {
            qWarning() << "Reading deprecated X-KDE-PluginInfo-Name property from ExternalAppModule, use Name property instead";
        }
    }
    externalModule.LblText->setText(i18n("%1 is an external application and has been automatically launched", moduleName));
    externalModule.PbRelaunch->setText(i18n("Relaunch %1", moduleName));
    connect(externalModule.PbRelaunch, &QPushButton::clicked, this, &ExternalAppModule::runExternal);
}

ExternalAppModule::~ExternalAppModule()
{
}

void ExternalAppModule::showEvent(QShowEvent *event)
{
    if (firstShow) {
        runExternal();
        firstShow = false;
    }
    QWidget::showEvent(event);
}

void ExternalAppModule::runExternal()
{
    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(module);
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
    job->start();
}
