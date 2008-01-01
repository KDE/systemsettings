/* This file is part of the KDE project
   Copyright 2007 Will Stephenson <wstephenson@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy 
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KCMODULEMODEL_H
#define KCMODULEMODEL_H

#include <QAbstractItemModel>
#include <kcategorizedsortfilterproxymodel.h>

class MenuItem;
class KCModuleModelPrivate;

class SystemSettingsProxyModel : public KCategorizedSortFilterProxyModel
{
public:
    SystemSettingsProxyModel(QObject *parent = 0);
    virtual ~SystemSettingsProxyModel();
    virtual bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const;
    virtual bool filterAcceptsRow( int source_column, const QModelIndex & source_parent ) const;
};

class KCModuleModel : public QAbstractItemModel
{
Q_OBJECT
public:
    static const int UserFilterRole, WeightRole;
    KCModuleModel( MenuItem * menuRoot, QObject * parent = 0 );
    ~KCModuleModel();
    // setup method
    // QAbstractItemModel reimplementations
    QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex & index ) const;
    int rowCount( const QModelIndex & index ) const;
    int columnCount( const QModelIndex & index ) const;

    QVariant data(const QModelIndex &index, int role) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
private:
    KCModuleModelPrivate * d;
};

class KCModuleSortFilterProxyModel : public KCategorizedSortFilterProxyModel
{
public:
protected:
};

#endif
