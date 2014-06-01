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

#include "Category.h"
#include "MenuProxyModel.h"

#include <QDebug>

class CategoryPrivate
{
public:
    CategoryPrivate(Category *host)
        : q(host)
    {
    }

    Category *q;

    QModelIndex modelIndex;
    //QAbstractItemModel *categoriesModel;
    QList<Category*> categories;
};

Category::Category(QModelIndex index, QObject *parent) :
    QObject(parent),
    d(new CategoryPrivate(this))
{
    d->modelIndex = index;
}

QQmlListProperty<Category> Category::categories()
{
    if (!d->categories.count()) {
        const int n = d->modelIndex.model()->rowCount(d->modelIndex);
        qDebug() << "ROWS:" << name() << n;
        for (int i = 0; i < n; i++) {
            QModelIndex index = d->modelIndex.child(i, 0);
            Category *category = new Category(index, this);
            d->categories.append(category);
            //QString c = d->modelIndex.model()->data(index, Qt::DisplayRole).toString();
            QString c = d->modelIndex.model()->data(index, Qt::DecorationRole).toString();
            qDebug() << " Cat from model: " << c << category->name();
        }

    }
    return QQmlListProperty<Category>(this, d->categories);
}


QVariant Category::decoration() const
{
    return d->modelIndex.model()->data(d->modelIndex, Qt::DecorationRole);
}

QString Category::name() const
{
    return d->modelIndex.model()->data(d->modelIndex, Qt::DisplayRole).toString();
}


Category::~Category()
{
    delete d;
}


