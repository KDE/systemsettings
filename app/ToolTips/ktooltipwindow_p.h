/***************************************************************************
 *   Copyright (C) 2009 by Peter Penz <peter.penz@gmx.at>                  *
 *   Copyright (C) 2012 by Mark Gaiser <markg85@gmail.com>                 *
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

#ifndef KTOOLTIPWINDOW_H
#define KTOOLTIPWINDOW_H

#include <QWidget>
class QPaintEvent;

class KToolTipWindow : public QWidget
{
    Q_OBJECT

public:
    explicit KToolTipWindow(QWidget* content);
    virtual ~KToolTipWindow();

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void showEvent(QShowEvent *);
};

#endif
