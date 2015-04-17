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
    Q_UNUSED( sortRole )

    painter->setRenderHint(QPainter::Antialiasing);

    const QRect optRect = option.rect;
    QFont font(QApplication::font());
    font.setBold(true);
    const QFontMetrics fontMetrics = QFontMetrics(font);
    
    const int height = categoryHeight(index, option);
    const QRect headerRect(optRect.left()+4, optRect.top(), optRect.width()-8, height-6);

    //BEGIN: draw text
    {
        const QString category = index.model()->data(index, KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();
        QRect textRect = headerRect.adjusted(0,0,0,-4);
        textRect.setLeft(textRect.left() + 4 /* a bit of margin */);
        painter->save();
        painter->setFont(font);
        QColor penColor(option.palette.text().color());
        painter->setPen(penColor);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, category);
//DEBUG         painter->drawRect(textRect); 
        painter->restore();
    }
    //END: draw text
    
    //BEGIN: draw separator
    {
        painter->save();
        
        QColor penColor(option.palette.highlight().color());
        painter->setPen(penColor);
        painter->drawLine(headerRect.bottomLeft(), headerRect.bottomRight());
        painter->restore();
    }
    //END: draw separator
}

int CategoryDrawer::categoryHeight(const QModelIndex &index, const QStyleOption &option) const
{
    Q_UNUSED( index );
    Q_UNUSED( option );

    QFont font(QApplication::font());
    font.setBold(true);
    const QFontMetrics fontMetrics = QFontMetrics(font);


    return fontMetrics.height() + 12 /* vertical spacing */;
}

int CategoryDrawer::leftMargin() const
{
    return 0;
}

int CategoryDrawer::rightMargin() const
{
    return 0;
}
