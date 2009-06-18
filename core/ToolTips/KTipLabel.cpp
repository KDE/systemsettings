/***************************************************************************
 *   Copyright (C) 2008 by Fredrik Höglund <fredrik@kde.org>               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "KTipLabel.h"

#include "KToolTipManager.h"

#ifdef Q_WS_X11
    #include <QX11Info>
#endif

KTipLabel::KTipLabel() : QWidget(0, Qt::ToolTip)
{
#ifdef Q_WS_X11
    if (QX11Info::isCompositingManagerRunning()) {
        setAttribute(Qt::WA_TranslucentBackground);
    }
#endif
}

void KTipLabel::showTip(const QPoint &pos, const KToolTipItem *item)
{
    currentItem = item;
    move(pos);
    show();
}

void KTipLabel::hideTip()
{
    hide();
    currentItem = 0;
}

void KTipLabel::moveTip(const QPoint &pos)
{
    move(pos);
}

void KTipLabel::paintEvent(QPaintEvent*)
{
    KStyleOptionToolTip option = styleOption();
    option.rect = rect();

    /** FIXME: Find out how to do this using Qt directly.
    if (QX11Info::isCompositingManagerRunning())
        XShapeCombineRegion(x11Info().display(), winId(), ShapeInput, 0, 0, delegate()->inputShape(&option).handle(), ShapeSet);
    else */
    setMask(delegate()->shapeMask(&option));

    QPainter p(this);
    p.setFont(option.font);
    p.setPen(QPen(option.palette.brush(QPalette::Text), 0));
    if( currentItem ) {
        delegate()->paint(&p, &option, currentItem);
    }
}

QSize KTipLabel::sizeHint() const
{
    if (!currentItem)
        return QSize();

    KStyleOptionToolTip option = styleOption();
    return delegate()->sizeHint(&option, currentItem);
}

KStyleOptionToolTip KTipLabel::styleOption() const
{
     KStyleOptionToolTip option;
     KToolTipManager::instance()->initStyleOption(&option);
     return option;
}

KToolTipDelegate *KTipLabel::delegate() const
{
    return KToolTipManager::instance()->delegate();
}

#include "KTipLabel.moc"
