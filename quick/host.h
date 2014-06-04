/***************************************************************************
 *                                                                         *
 *   Copyright 2014 Sebastian Kügler <sebas@kde.org>                       *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef HOST_H
#define HOST_H

#include "Category.h"

#include <QAbstractItemModel>
#include <QObject>
#include <QQmlListProperty>

class HostPrivate;
class MenuProxyModel;
class QuickMode;

class Host : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel *categoriesModel READ categoriesModel CONSTANT)
    Q_PROPERTY(QQmlListProperty<Category> categories READ categories CONSTANT)
    Q_PROPERTY(QQmlListProperty<Category> modules READ modules NOTIFY modulesChanged)
    Q_PROPERTY(bool moduleWidgetVisible READ moduleWidgetVisible WRITE setModuleWidgetVisible NOTIFY moduleWidgetVisibleChanged)

public:
    Host();
    virtual ~Host();

    static Host* self();

    void setModel(MenuProxyModel *model);
    void setQuickMode(QuickMode *quickmode);
    void selectModule(Category *cat);

    QQmlListProperty<Category> categories();
    QQmlListProperty<Category> modules();

public Q_SLOTS:
    QAbstractItemModel *categoriesModel();
    Q_INVOKABLE void categoryClicked(int ix);
    Q_INVOKABLE void resetModules();
    Q_INVOKABLE void moduleClicked(int ix);

    Q_INVOKABLE void setColumnWidth(int col, int colWidth);
    Q_INVOKABLE void setRowHeight(int row, int rowHeight);

    bool moduleWidgetVisible();
    void setModuleWidgetVisible(bool vis);

Q_SIGNALS:
    void moduleSelected(QModelIndex);
    void modulesChanged();
    void moduleWidgetVisibleChanged();

private:
    HostPrivate *d;
};

#endif // HOST_H
