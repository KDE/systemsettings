/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef EXTERNALAPPMODULE_H
#define EXTERNALAPPMODULE_H

#include <QWidget>

#include "ui_externalModule.h"

class QShowEvent;
class KCModuleInfo;

class ExternalAppModule : public QWidget
{
    Q_OBJECT

public:
    explicit ExternalAppModule(QWidget *parent = nullptr, KCModuleInfo *module = nullptr);
    ~ExternalAppModule() override;

protected:
    void showEvent(QShowEvent *event) override;

private Q_SLOTS:
    void runExternal();

private:
    KCModuleInfo *moduleInfo;
    Ui::ExternalModule externalModule;
    bool firstShow;
};

#endif
