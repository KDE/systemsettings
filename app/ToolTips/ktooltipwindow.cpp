/*******************************************************************************
 *   Copyright (C) 2008 by Fredrik Höglund <fredrik@kde.org>                   *
 *   Copyright (C) 2008 by Konstantin Heil <konst.heil@stud.uni-heidelberg.de> *
 *   Copyright (C) 2009 by Peter Penz <peter.penz@gmx.at>                      *
 *                                                                             *
 *   This program is free software; you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by      *
 *   the Free Software Foundation; either version 2 of the License, or         *
 *   (at your option) any later version.                                       *
 *                                                                             *
 *   This program is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 *   GNU General Public License for more details.                              *
 *                                                                             *
 *   You should have received a copy of the GNU General Public License         *
 *   along with this program; if not, write to the                             *
 *   Free Software Foundation, Inc.,                                           *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA                *
 *******************************************************************************/

#include "ktooltipwindow_p.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QStylePainter>
#include <QStyleOptionFrame>

KToolTipWindow::KToolTipWindow(QWidget* content) :
    QWidget(0)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(content);
}

KToolTipWindow::~KToolTipWindow()
{
}

void KToolTipWindow::paintEvent(QPaintEvent* event)
{
   QStylePainter painter(this);
   QStyleOptionFrame option;
   option.init(this);
   painter.drawPrimitive(QStyle::PE_PanelTipLabel, option);
   painter.end();

   QWidget::paintEvent(event);
}

#include "ktooltipwindow_p.moc"
