/**************************************************************************
 * Copyright (C) 2009 Ben Cooksley <bcooksley@kde.org>                    *
 *                                                                        *
 * This program is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU General Public License            *
 * as published by the Free Software Foundation; either version 2         *
 * of the License, or (at your option) any later version.                 *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA          *
 * 02110-1301, USA.                                                       *
***************************************************************************/
#ifndef CLASSICMODE_H
#define CLASSICMODE_H

#include "BaseMode.h"

class MenuItem;
class ModuleView;
class QModelIndex;

class ClassicMode : public BaseMode
{
    Q_OBJECT

public:
    ClassicMode(QObject * parent, const QVariantList& );
    ~ClassicMode() override;
    void initEvent() override;
    void leaveModuleView() override;
    QWidget * mainWidget() override;
    KAboutData * aboutData() override;
    ModuleView * moduleView() const override;

protected:
    QList<QAbstractItemView*> views() const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

public Q_SLOTS:
    void expandColumns();
    void searchChanged( const QString& text ) override;
    void selectModule( const QModelIndex& selectedModule );
    void changeModule( const QModelIndex& activeModule );
    void saveState() override;
    void giveFocus() override;
    void addConfiguration( KConfigDialog * config ) override;
    void loadConfiguration() override;
    void saveConfiguration() override;

private Q_SLOTS:
    void moduleLoaded();
    void initWidget();
    void moveUp( MenuItem * item );

private:
    class Private;
    Private *const d;
};

#endif
