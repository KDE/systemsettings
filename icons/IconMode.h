/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ICONMODE_H
#define ICONMODE_H

#include "BaseMode.h"

class ModuleView;
class KAboutData;
class QModelIndex;
class QAbstractItemView;

class IconMode : public BaseMode
{
    Q_OBJECT

public:
    explicit IconMode(QObject *parent, const QVariantList &);
    ~IconMode() override;
    QWidget *mainWidget() override;
    void initEvent() override;
    void giveFocus() override;
    void leaveModuleView() override;
    KAboutData *aboutData() override;
    ModuleView *moduleView() const override;
    void reloadStartupModule() override;

protected:
    QList<QAbstractItemView *> views() const override;
    bool eventFilter(QObject *watched, QEvent *event) override;

public Q_SLOTS:
    void searchChanged(const QString &text) override;

private Q_SLOTS:
    void moduleLoaded();
    void backToOverview();
    void initWidget();

private:
    void changeModule(const QModelIndex &activeModule);
    void changeModuleWithArgs(const QModelIndex &activeModule, const QStringList &args);

    class Private;
    Private *const d;
};

#endif
