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

#include "KToolTipManager.h"

#include <QToolTip>
#include <QApplication>

KToolTipManager *KToolTipManager::s_instance = 0;

KToolTipManager::KToolTipManager()
    : QSharedData(), label(new KTipLabel), currentItem(0), m_delegate(0)
{
}

KToolTipManager::~KToolTipManager()
{
    delete label;
    delete currentItem;
}

void KToolTipManager::showTip(const QPoint &pos, KToolTipItem *item)
{
    hideTip();
    label->showTip(pos, item);
    currentItem = item;
    m_tooltipPos = pos;
}

void KToolTipManager::hideTip()
{
    label->hideTip();
    delete currentItem;
    currentItem = 0;
}

void KToolTipManager::initStyleOption(KStyleOptionToolTip *option) const
{
    option->direction      = QApplication::layoutDirection();
    option->fontMetrics    = QFontMetrics(QToolTip::font());
    option->activeCorner   = KStyleOptionToolTip::TopLeftCorner;
    option->palette        = QToolTip::palette();
    option->font           = QToolTip::font();
    option->rect           = QRect();
    option->state          = QStyle::State_None;
    option->decorationSize = QSize(32, 32);
}

void KToolTipManager::setDelegate(KToolTipDelegate *delegate)
{
    m_delegate = delegate;
}

void KToolTipManager::update()
{
    if (currentItem == 0)
        return;
    label->showTip(m_tooltipPos, currentItem);
}

KToolTipDelegate *KToolTipManager::delegate() const
{
    return m_delegate;
}
