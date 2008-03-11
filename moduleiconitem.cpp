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

#include "moduleiconitem.h"
#include <kiconloader.h>
#include <kdebug.h>
#include <kcmoduleinfo.h>

#include <climits>

#include <QApplication>
#include <QPainter>

#define IMAGE_SIZE 32
#define ICON_WIDTH 100


ModuleIconItem::ModuleIconItem( QListWidget* parent, const KCModuleInfo& module)
	: QListWidgetItem(SmallIcon( module.icon(), IMAGE_SIZE ), module.moduleName(), parent),
	imageName(module.icon())
{
	setData( Qt::UserRole, KIconLoader::DefaultState );
	modules.append(module);
	setSize();
}

ModuleIconItem::ModuleIconItem( QListWidget* parent, const QString &text,
		const QString &_imageName )
	: QListWidgetItem( SmallIcon( _imageName, IMAGE_SIZE ), text, parent ),
	imageName(_imageName)
{
	setData( Qt::UserRole, KIconLoader::DefaultState );
	setSize();
}

void ModuleIconItem::loadIcon( bool enabled )
{
	int newState = enabled ? KIconLoader::DefaultState : KIconLoader::DisabledState;
	if( newState == data( Qt::UserRole ).toInt() )
		return;

	setData( Qt::UserRole, newState );
	setIcon( DesktopIcon( imageName, IMAGE_SIZE , newState ) );
}

void ModuleIconItem::setSize()
{
	QStyle *style = listWidget()->style();
	QFontMetrics fm(font());
	const QRect &rect = fm.boundingRect(0, 0, ICON_WIDTH, INT_MAX, Qt::TextWordWrap, text());
	setData(Qt::SizeHintRole, QSize(ICON_WIDTH, IMAGE_SIZE + style->pixelMetric(QStyle::PM_FocusFrameVMargin) + rect.height()));
}
