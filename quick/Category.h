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

#ifndef CATEGORY_H
#define CATEGORY_H

#include <QAbstractItemModel>
#include <QObject>
#include <QQmlListProperty>


class CategoryPrivate;
class MenuProxyModel;

class Category : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<Category> categories READ categories CONSTANT)
    Q_PROPERTY(QVariant decoration READ decoration CONSTANT);
    Q_PROPERTY(QString name READ name CONSTANT);

public:
    Category(QModelIndex index, QObject *parent = 0);
    virtual ~Category();

    QQmlListProperty<Category> categories();
    QVariant decoration() const;
    QString name() const;

public Q_SLOTS:

Q_SIGNALS:
    void selected(QModelIndex);


private:
    CategoryPrivate *d;
};

#endif // CATEGORY_H
