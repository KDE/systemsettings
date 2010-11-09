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

#include "MenuProxyModel.h"

#include "MenuItem.h"
#include "MenuModel.h"

MenuProxyModel::MenuProxyModel( QObject * parent )
    : KCategorizedSortFilterProxyModel( parent )
{
    setSortRole( MenuModel::UserSortRole );
    setFilterRole( MenuModel::UserFilterRole );
    setFilterCaseSensitivity( Qt::CaseInsensitive );
}

bool MenuProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
    if( isCategorizedModel() ) {
        return KCategorizedSortFilterProxyModel::lessThan( left, right );
    }
    
    QVariant leftWeight = left.data( MenuModel::UserSortRole );
    QVariant rightWeight = right.data( MenuModel::UserSortRole );

    if ( leftWeight.toInt() == rightWeight.toInt() ) {
        return left.data().toString() < right.data().toString();
    }

    return leftWeight.toInt() < rightWeight.toInt();
}

bool MenuProxyModel::subSortLessThan( const QModelIndex &left, const QModelIndex &right ) const
{
    if( isCategorizedModel() ) {
        QVariant leftWeight = left.data( MenuModel::UserSortRole );
        QVariant rightWeight = right.data( MenuModel::UserSortRole );

        if ( !leftWeight.isValid() || !rightWeight.isValid() ) {
            return KCategorizedSortFilterProxyModel::subSortLessThan( left, right );
        } else {
            if ( leftWeight.toInt() == rightWeight.toInt() ) {
                return left.data().toString() < right.data().toString();
            } else {
                return leftWeight.toInt() < rightWeight.toInt();
            }
        }
     }
     return KCategorizedSortFilterProxyModel::subSortLessThan( left, right );
}


bool MenuProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
    QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
    MenuItem * mItem = index.data( Qt::UserRole ).value<MenuItem*>();
    // accept only systemsettings categories that have children
    if ( mItem->children().isEmpty() && mItem->service()->serviceTypes().contains( "SystemSettingsCategory" ) ) {
        return false;
    } else {
        return true; // Items matching the regexp are disabled, not hidden
    }
}

Qt::ItemFlags MenuProxyModel::flags( const QModelIndex &index ) const
{
    if ( !index.isValid() ) {
        return Qt::NoItemFlags;
    }

    QString matchText = index.data( MenuModel::UserFilterRole ).toString();
    if( !matchText.contains( filterRegExp() ) ) {
        return Qt::NoItemFlags;
    } else {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
}

void MenuProxyModel::setFilterRegExp ( const QString & pattern )
{
    emit layoutAboutToBeChanged ();
    KCategorizedSortFilterProxyModel::setFilterRegExp( pattern );
    emit layoutChanged ();
}

void MenuProxyModel::setFilterRegExp ( const QRegExp & regExp )
{
    emit layoutAboutToBeChanged ();
    KCategorizedSortFilterProxyModel::setFilterRegExp( regExp );
    emit layoutChanged ();
}


#include "MenuProxyModel.moc"
