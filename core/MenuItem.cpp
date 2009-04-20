/* This file is part of the KDE project
   Copyright 2007 Will Stephenson <wstephenson@kde.org>
   Copyright 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

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

#include "MenuItem.h"

#include <QList>
#include <QString>

#include <KCModuleInfo>

class MenuItem::Private {
public:
    Private() {}

    MenuItem *parent;
    QList<MenuItem*> children;
    QList<MenuItem*> grandChildren;
    bool menu;
    QString name;
    KService::Ptr service;
    KCModuleInfo item;
};

MenuItem::MenuItem( bool isMenu, MenuItem * itsParent )
    : d( new Private() )
{
    d->parent = itsParent;
    d->menu = isMenu;

    if ( d->parent ) {
        d->parent->children().append( this );
        MenuItem * grandParent = d->parent->parent();
        if( grandParent ) {
            grandParent->grandChildren().append(this);
        }
    }
}

MenuItem::~MenuItem()
{
    qDeleteAll( d->children );
    d->grandChildren.clear();
    delete d;
}

inline int weightOfService( const KService::Ptr service )
{
    QVariant tmp = service->property( "X-KDE-Weight", QVariant::Int );
    return ( tmp.isValid() ? tmp.toInt() : 100 );
}

static bool childIsLessThan( MenuItem *left, MenuItem *right )
{
    return weightOfService( left->service() ) < weightOfService( right->service() );
}

void MenuItem::sortChildrenByWeight()
{
    qSort( d->children.begin(), d->children.end(), childIsLessThan );
}

MenuItem * MenuItem::child( int index )
{
    return d->children.at(index);
}

MenuItem * MenuItem::grandChild( int index )
{
    return d->grandChildren.at(index);
}

QStringList MenuItem::keywords()
{
    QStringList listOfKeywords;

    listOfKeywords << d->item.keywords() << d->service->name();
    foreach ( MenuItem * child, d->children ) {
        listOfKeywords += child->keywords();
    }
    return listOfKeywords;
}

MenuItem* MenuItem::parent() const
{
    return d->parent;
}

QList<MenuItem*>& MenuItem::children() const
{
    return d->children;
}

QList<MenuItem*>& MenuItem::grandChildren() const
{
    return d->grandChildren;
}

KService::Ptr& MenuItem::service() const
{
    return d->service;
}

KCModuleInfo& MenuItem::item() const
{
    return d->item;
}

QString& MenuItem::name() const
{
    return d->name;
}

bool MenuItem::menu() const
{
    return d->menu;
}

void MenuItem::setService( const KService::Ptr& service )
{
    d->service = service;
    d->name = service->property("X-KDE-System-Settings-Category").toString();
    d->item = KCModuleInfo( service->entryPath() );
}
