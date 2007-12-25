/**
 * This file is part of the System Settings package
 * Copyright (C) 2005 Benjamin C Meyer
 *                    <ben+systempreferences at meyerhome dot net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef MODULEICONITEM_H
#define MODULEICONITEM_H

#include <QItemDelegate>
#include <QListWidgetItem>
#include <QList>
#include <QPainterPath>
#include <QRectF>

class KCModuleInfo;

class ModuleIconItemDelegate : public QItemDelegate
{
	public:
		ModuleIconItemDelegate(QObject *parent);
		
		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	private:
		// Method taken from KFileItemDelegate. Check whether it has been moved to
		// kdefx/kdrawutil.cpp as the comment says on Fredrik's code. If so, remove
		// this code (duplication), and use the library one.
		QPainterPath roundedRectangle(const QRectF &rect, qreal radius) const;
};

/**
 * Stores information about what modules goes with this item.
 * Also provides means of loading the enabled/disabled image (see kcmsearch).
 */
class ModuleIconItem : public QListWidgetItem
{

public:
	ModuleIconItem( QListWidget *parent, const KCModuleInfo& module );
	
	ModuleIconItem( QListWidget *parent, const QString &text, const QString &imageName );

	/**
	 * Update the icon to either be enabled or not.
	 */
	void loadIcon( bool enabled = true );

	// The modules that go with this item
	QList<KCModuleInfo> modules;
					
private:
	void setSize();

	QString imageName;
};

#endif // MODULEICONITEM_H

