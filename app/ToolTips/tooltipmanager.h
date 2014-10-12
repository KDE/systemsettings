/*******************************************************************************
 *   Copyright (C) 2008 by Konstantin Heil <konst.heil@stud.uni-heidelberg.de> *
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

#ifndef TOOLTIPMANAGER_H
#define TOOLTIPMANAGER_H

#include <QtCore/QObject>

#include <QtCore/QModelIndex>

class QLayout;
class QAbstractItemView;

/**
 * @brief Manages the tooltips for an item view.
 *
 * When hovering an item, a tooltip is shown after
 * a short timeout. The tooltip is hidden again when the
 * viewport is hovered or the item view has been left.
 */
class ToolTipManager : public QObject
{
    Q_OBJECT

public:
    /**
    * Standard constructor. The ToolTipManager will start handling ToolTip events on the provided
    * view immediately.
    *
    * @param parent The view which will have the tooltips displayed for.
    */
    explicit ToolTipManager(QAbstractItemView* parent);
    virtual ~ToolTipManager();

public Q_SLOTS:
    /**
     * Hides the currently shown tooltip. Invoking this method is
     * only needed when the tooltip should be hidden although
     * an item is hovered.
     */
    void hideToolTip();

protected:
    /**
    * Please see the Qt documentation for more details.
    *
    * @param watched The object that was being watched.
    * @param event The event object.
    * @returns true if the event was handled in this filter, or false if it was not.
    */
    virtual bool eventFilter( QObject* watched, QEvent* event );

private Q_SLOTS:
    void prepareToolTip();
    void requestToolTip(const QModelIndex& index);

private:
    void showToolTip( QModelIndex menuItem );
    QWidget * createTipContent( QModelIndex item );
    QLayout * generateToolTipLine( QModelIndex * item, QWidget * toolTip, QSize iconSize, bool comment );

    class Private;
    ToolTipManager::Private* d;
};

#endif
