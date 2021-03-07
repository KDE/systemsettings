/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ExternalAppModule.h"

#include <KCModuleInfo>
#include <KIO/ApplicationLauncherJob>
#include <KIO/JobUiDelegate>

ExternalAppModule::ExternalAppModule(QWidget *parent, KCModuleInfo *module)
{
    Q_UNUSED(parent)

    firstShow = true;
    moduleInfo = module;
    externalModule.setupUi(this);
    externalModule.LblText->setText(i18n("%1 is an external application and has been automatically launched", module->moduleName()));
    externalModule.PbRelaunch->setText(i18n("Relaunch %1", module->moduleName()));
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
    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(moduleInfo->service());
    job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
    job->start();
}
