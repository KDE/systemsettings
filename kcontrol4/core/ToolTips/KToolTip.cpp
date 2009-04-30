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

#include "KToolTip.h"

#include "KToolTipManager.h"

namespace KToolTip
{
    void showText(const QPoint &pos, const QString &text, QWidget *widget, const QRect &rect)
    {
        Q_UNUSED(widget)
        Q_UNUSED(rect)
        KToolTipItem *item = new KToolTipItem(text);
        KToolTipManager::instance()->showTip(pos, item);
    }

    void showText(const QPoint &pos, const QString &text, QWidget *widget)
    {
        showText(pos, text, widget, QRect());
    }

    void showTip(const QPoint &pos, KToolTipItem *item)
    {
        KToolTipManager::instance()->showTip(pos, item);
    }

    void hideTip()
    {
        KToolTipManager::instance()->hideTip();
    }

    void setToolTipDelegate(KToolTipDelegate *delegate)
    {
        KToolTipManager::instance()->setDelegate(delegate);
    }
}
