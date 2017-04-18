/***************************************************************************
 *   Copyright (C) 2009 by Rafael Fernández López <ereslibre@kde.org>      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "CategoryDrawer.h"

#include "MenuProxyModel.h"

#include <QPainter>
#include <QApplication>
#include <QStyleOption>
#include <QDebug>

CategoryDrawer::CategoryDrawer(KCategorizedView *view)
: KCategoryDrawer(view)
{
}

void CategoryDrawer::drawCategory(const QModelIndex &index,
                                            int sortRole,
                                            const QStyleOption &option,
                                            QPainter *painter) const
{
    Q_UNUSED( option )
    Q_UNUSED( painter )
    Q_UNUSED( sortRole )

    painter->setRenderHint(QPainter::Antialiasing);

    const QRect optRect = option.rect;
    QFont font(QApplication::font());
    font.setBold(true);
    const QFontMetrics fontMetrics = QFontMetrics(font);
    const int height = categoryHeight(index, option);


    const QString category = index.model()->data(index, KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();
    QRect textRect = QRect(option.rect.topLeft()+QPoint(8,0), QSize(option.rect.width() - 2 - 3 - 3, height));

    textRect.setLeft(textRect.left());
    painter->save();
    painter->setFont(font);
    QColor penColor(option.palette.text().color());
    penColor.setAlphaF(0.6);
    painter->setPen(penColor);
    if (index.row() > 0) {
        textRect.setTop(textRect.top() + 10);
        painter->save();
        penColor.setAlphaF(0.3);
        painter->fillRect(QRect(textRect.topLeft() + QPoint(-8, -5), QSize(option.rect.width(),1)), penColor);
        painter->restore();
    }

    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignBottom, category);
    painter->restore();

}

int CategoryDrawer::categoryHeight(const QModelIndex &index, const QStyleOption &option) const
{
    Q_UNUSED( index );
    Q_UNUSED( option );

    QFont font(QApplication::font());
    font.setBold(true);
    const QFontMetrics fontMetrics = QFontMetrics(font);
    //if (index.row() == 0) return fontMetrics.height();

    return fontMetrics.height() * 1.6  /* vertical spacing */;
}

int CategoryDrawer::leftMargin() const
{
    return 0;
}

int CategoryDrawer::rightMargin() const
{
    return 0;
}
