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

#include "CategorizedView.h"

#include <KFileItemDelegate>

#include <QScrollBar>

CategorizedView::CategorizedView( QWidget *parent )
    : KCategorizedView( parent )
{
    setWordWrap( true );
    setViewportMargins(QMargins(0,0,-20,0));
}

void CategorizedView::setModel( QAbstractItemModel *model )
{
    KCategorizedView::setModel( model );
}

void CategorizedView::wheelEvent(QWheelEvent* event)
{
    // this is a workaround because scrolling by mouse wheel is broken in Qt list views for big items
    // https://bugreports.qt-project.org/browse/QTBUG-7232
    verticalScrollBar()->setSingleStep(10);
    KCategorizedView::wheelEvent(event);
}
