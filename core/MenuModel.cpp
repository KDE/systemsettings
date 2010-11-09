/**************************************************************************
 * Copyright (C) 2009 Ben Cooksley <bcooksley@kde.org>                    *
 * Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>               *
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

#include "MenuModel.h"

#include <KIcon>
#include <KCategorizedSortFilterProxyModel>

#include "MenuItem.h"

const int MenuModel::UserFilterRole = 0x015D1AE6;
const int MenuModel::UserSortRole = 0x03A8CC00;

class MenuModel::Private {
public:
    Private() {}

    MenuItem *rootItem;
    QList<MenuItem*> exceptions;
};

MenuModel::MenuModel( MenuItem * menuRoot, QObject *parent )
    : QAbstractItemModel( parent )
    , d( new Private() )
{
    d->rootItem = menuRoot;
}

MenuModel::~MenuModel()
{
    d->exceptions.clear();
    delete d;
}

int MenuModel::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return 1;
}

int MenuModel::rowCount( const QModelIndex &parent ) const
{
    MenuItem * mi;
    if ( parent.isValid() ) {
        mi = static_cast<MenuItem*>( parent.internalPointer() );
    } else {
        mi = d->rootItem;
    }

    return childrenList(mi).count();
}

QVariant MenuModel::data( const QModelIndex &index, int role ) const
{
    MenuItem * mi = 0;
    QVariant theData;
    if ( !index.isValid() ) {
        return QVariant();
    }

    mi = static_cast<MenuItem*>( index.internalPointer() );
    switch ( role ) {
        case Qt::DisplayRole:
            theData.setValue( mi->name() );
            break;
        case Qt::ToolTipRole:
            theData.setValue( mi->service()->comment() );
            break;
        case Qt::DecorationRole:
            theData = QVariant( KIcon( mi->service()->icon() ) );
            break;
        case KCategorizedSortFilterProxyModel::CategorySortRole:
            if ( mi->parent() ) {
                theData.setValue( QString("%1%2").arg( QString::number(mi->parent()->weight()), 5, '0' ).arg( mi->parent()->name() ) );
            }
            break;
        case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
            if ( mi->parent() ) {
                theData.setValue( mi->parent()->name() );
            }
            break;
        case Qt::UserRole:
            theData.setValue( mi );
            break;
        case MenuModel::UserFilterRole:
            theData.setValue( mi->keywords().join( QString() ) );
            break;
        case MenuModel::UserSortRole:
            theData.setValue( QString("%1").arg( QString::number(mi->weight()), 5, '0' ) );
            break;
        default:
            break;
    }
    return theData;
}

Qt::ItemFlags MenuModel::flags( const QModelIndex &index ) const
{
    if ( !index.isValid() ) {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex MenuModel::index( int row, int column, const QModelIndex &parent ) const
{
    if ( !hasIndex(row, column, parent) ) {
        return QModelIndex();
    }

    MenuItem *parentItem;
    if ( !parent.isValid() ) {
        parentItem = d->rootItem;
    } else {
        parentItem = static_cast<MenuItem*>( parent.internalPointer() );
    }

    MenuItem *childItem = childrenList(parentItem).value(row);
    if ( childItem ) {
        return createIndex( row, column, childItem );
    } else {
        return QModelIndex();
    }
}

QModelIndex MenuModel::parent( const QModelIndex &index ) const
{
    MenuItem *childItem = static_cast<MenuItem*>( index.internalPointer() );
    if( !childItem ) {
        return QModelIndex();
    }

    MenuItem * parent = parentItem(childItem);
    MenuItem * grandParent = parentItem(parent);

    int childRow = 0;
    if( grandParent ) {
        childRow = childrenList( grandParent ).indexOf( parent );
    }

    if ( parent == d->rootItem ) {
        return QModelIndex();
    }
    return createIndex( childRow, 0, parent );
}

QList<MenuItem*> MenuModel::childrenList( MenuItem * parent ) const
{
    QList<MenuItem*> children = parent->children();
    foreach( MenuItem * child, children ) {
        if( d->exceptions.contains( child ) ) {
            children.removeOne(child);
            children.append(child->children());
        }
    }
    return children;
}

MenuItem * MenuModel::parentItem( MenuItem * child ) const
{
    MenuItem * parent = child->parent();
    if( d->exceptions.contains(parent) ) {
        parent = parentItem(parent);
    }
    return parent;
}

MenuItem * MenuModel::rootItem() const
{
    return d->rootItem;
}

void MenuModel::addException( MenuItem * exception )
{
    if( exception == d->rootItem ) {
        return;
    }
    d->exceptions.append(exception);
}

void MenuModel::removeException( MenuItem * exception )
{
    d->exceptions.removeAll(exception);
}

#include "MenuModel.moc"
