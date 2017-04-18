/**************************************************************************
 * Copyright (C) 2017 Marco Martin <mart@kde.org>                         *
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

#ifndef SIDEBARDELEGATE_H
#define SIDEBARDELEGATE_H

#include <QAbstractItemDelegate>

class SidebarDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    SidebarDelegate(QObject *parent = nullptr);
    ~SidebarDelegate();


    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
    QBrush foregroundBrush(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif

