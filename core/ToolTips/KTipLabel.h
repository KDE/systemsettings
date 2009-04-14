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

#ifndef KTIPLABEL_H
#define KTIPLABEL_H

#include <QWidget>

#include "KToolTipItem.h"
#include "KToolTipDelegate.h"
#include "KStyleOptionToolTip.h"

#include <QSize>
#include <QPoint>
#include <QPaintEvent>

class KTipLabel : public QWidget
{
    Q_OBJECT
public:
    KTipLabel();
    void showTip(const QPoint &pos, const KToolTipItem *item);
    void moveTip(const QPoint &pos);
    void hideTip();

private:
    void paintEvent(QPaintEvent*);
    QSize sizeHint() const;
    KStyleOptionToolTip styleOption() const;
    KToolTipDelegate *delegate() const;

private:
    const KToolTipItem *currentItem;
};

#endif
