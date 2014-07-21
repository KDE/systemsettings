/* This file is part of the KDE project
   Copyright 2007 Will Stephenson <wstephenson@kde.org>
   Copyright (C) 2009 Ben Cooksley <bcooksley@kde.org> 

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

static bool childIsLessThan( MenuItem *left, MenuItem *right )
{
    return left->weight() < right->weight();
}

class MenuItem::Private {
public:
    Private() {}

    MenuItem *parent;
    QList<MenuItem*> children;
    bool menu;
    QString name;
    QString category;
    int weight;
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
    }
}

MenuItem::~MenuItem()
{
    qDeleteAll( d->children );
    delete d;
}

void MenuItem::sortChildrenByWeight()
{
    qSort( d->children.begin(), d->children.end(), childIsLessThan );
}

MenuItem * MenuItem::child( int index )
{
    return d->children.at(index);
}

QStringList MenuItem::keywords()
{
    QStringList listOfKeywords;

    listOfKeywords << d->item.keywords() << d->name;
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

QString& MenuItem::category() const
{
    return d->category;
}

int MenuItem::weight()
{
    return d->weight;
}

bool MenuItem::menu() const
{
    return d->menu;
}

void MenuItem::setService( const KService::Ptr& service )
{
    d->service = service;
    d->category = service->property("X-KDE-System-Settings-Category").toString();
    d->name = service->name();
    d->item = KCModuleInfo( service );
    const QVariant itemWeight = service->property( "X-KDE-Weight", QVariant::Int );
    if( itemWeight.isValid() ) {
        d->weight = itemWeight.toInt();
    } else {
        d->weight = 100;
    }
}
