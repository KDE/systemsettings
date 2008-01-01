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

#include "kcmodulemodel.h"

#include <QHash>
#include <QList>
#include <KDebug>
#include <KIcon>
#include <KServiceTypeTrader>

#include "kcategorizedsortfilterproxymodel.h"
#include "menuitem.h"

Q_DECLARE_METATYPE(MenuItem *)

SystemSettingsProxyModel::SystemSettingsProxyModel( QObject * parent )
    : KCategorizedSortFilterProxyModel( parent )
{

}

SystemSettingsProxyModel::~SystemSettingsProxyModel()
{}

bool SystemSettingsProxyModel::subSortLessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant leftWeight = left.data( KCModuleModel::WeightRole );
    QVariant rightWeight = right.data( KCModuleModel::WeightRole );

    if ( !( leftWeight.isValid() && rightWeight.isValid() ) ) {
        return KCategorizedSortFilterProxyModel::subSortLessThan( left, right );
    } else {
        kDebug() << "comparing " << left.data().toString() << " (" << leftWeight.toInt() << ") and " << right.data().toString() << " (" << rightWeight.toInt() << ")";
        if ( leftWeight.toInt() == rightWeight.toInt() ) {
            return left.data().toString() < right.data().toString();
        } else {
            return leftWeight.toInt() < rightWeight.toInt();
        }
    }
}


bool SystemSettingsProxyModel::filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const
{
    QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
    MenuItem * mItem = index.data( Qt::UserRole ).value<MenuItem*>();
    // accept either items with children or those that have valid KCModuleInfo
    return !( mItem->children.isEmpty() && mItem->item.service().isNull() );
}

int weightOfService( const KService::Ptr service )
{
    QVariant tmp = service->property( "X-KDE-Weight", QVariant::Int );
    int weight = tmp.isValid() ? tmp.toInt() : 100;
    return weight;
}

class KCModuleModelPrivate {
public:
    KCModuleModelPrivate(){
    }

    MenuItem * rootItem;
};

const int KCModuleModel::UserFilterRole = 0x015D1AE6;
const int KCModuleModel::WeightRole = 0x03A8CC00;

KCModuleModel::KCModuleModel( MenuItem * menuRoot, QObject * parent )
 : QAbstractItemModel( parent ), d( new KCModuleModelPrivate )
{
    d->rootItem = menuRoot;
}

KCModuleModel::~KCModuleModel()
{
}

int KCModuleModel::rowCount( const QModelIndex & index ) const
{
    int count = 0;
    MenuItem * mi;
    if ( index.isValid() ) {
        mi = static_cast<MenuItem *>( index.internalPointer() );
    } else {
        mi = d->rootItem;
    }
    if ( mi ) {
        foreach ( MenuItem * i, mi->children ) {
            count += i->children.count();
        }
    }
    return count;
}

int KCModuleModel::columnCount( const QModelIndex & /*index*/ ) const
{
    return 2; // name and comment
}

QModelIndex KCModuleModel::index( int row, int column, const QModelIndex & parent ) const
{
    if ( !hasIndex( row, column, parent ) ) {
        return QModelIndex();
    }
    MenuItem * parentItem;
    if ( !parent.isValid() ) {
        parentItem = d->rootItem;
    } else {
        parentItem = static_cast<MenuItem*>( parent.internalPointer() );
    }
    MenuItem * foundItem = parentItem->grandChildAt( row );
    if ( foundItem ) {
        QModelIndex index = createIndex( row, column, (void*)foundItem );
        return index;
    }
    return QModelIndex();
}


QModelIndex KCModuleModel::parent( const QModelIndex & index ) const
{
    if ( !index.isValid() ) {
        return QModelIndex();
    }

    MenuItem * parentItem = static_cast<MenuItem*>( index.internalPointer() )->parent;
    if ( parentItem == d->rootItem ) {
        return QModelIndex();
    } else {
        return createIndex( parentItem->parent->children.indexOf( parentItem ), 0, parentItem );
    }
}

QVariant KCModuleModel::data(const QModelIndex &index, int role) const
{
    MenuItem * mi = 0;
    QVariant theData;
    if ( !index.isValid() ) {
        return QVariant();
    }
    mi = static_cast<MenuItem *>( index.internalPointer() );
    QStringList searchKeyWords;
    switch ( role ) {
        case Qt::DisplayRole:
            switch ( index.column() ) {
                case 0:
                    theData.setValue( mi->service->name());
                    break;
                case 1:
                    theData.setValue( mi->service->comment());
                    break;
                default:
                    break;
            }
            break;
        case Qt::DecorationRole:
            if ( index.column() == 0 )
                theData = QVariant( KIcon( mi->service->icon() ) );
            break;
        case KCategorizedSortFilterProxyModel::CategorySortRole:
            if ( mi->parent )
                theData.setValue( QString("%1%2").arg( QString::number( weightOfService( mi->parent->service) ), 5, '0' ).arg( mi->parent->service->name() ) );
            break;
        case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
            if ( mi->parent )
                theData.setValue( mi->parent->service->name());
            break;
        case Qt::UserRole:
            theData.setValue( mi );
            break;
        case UserFilterRole:
            foreach ( MenuItem * child, mi->children ) {
                searchKeyWords << child->item.keywords() << child->service->name();
            }
            searchKeyWords << mi->item.keywords() << mi->service->name();
            theData.setValue( searchKeyWords.join( QString() ) );
            break;
        case WeightRole:
            theData.setValue( weightOfService( mi->service ) );
            break;
        default:
            break;
    }
    return theData;
}

Qt::ItemFlags KCModuleModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant KCModuleModel::headerData(int section, Qt::Orientation orientation, int role ) const
{
    return QAbstractItemModel::headerData( section, orientation, role );
}

