/***************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <bcooksley@kde.org>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

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
