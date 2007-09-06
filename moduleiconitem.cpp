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

#include <qapplication.h>
#include <qpainter.h>

#define IMAGE_SIZE 32
#define ICON_WIDTH 100

ModuleIconItemDelegate::ModuleIconItemDelegate(QObject *parent) : QItemDelegate(parent)
{
}

void ModuleIconItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyle *style;
	if (const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option))
		style = v3->widget->style();
	else
		style = QApplication::style();

	const QSize &decorationSize = option.decorationSize;
	const QPixmap &pixmap = qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).pixmap(option.decorationSize);
	int iconX = option.rect.left() + (option.rect.width() - decorationSize.width()) / 2;
	painter->drawPixmap(iconX, 0, decorationSize.width(), decorationSize.height(), pixmap);
	
	QRect textRectangle = option.rect;
	textRectangle.setTop(textRectangle.top() + decorationSize.height() + style->pixelMetric(QStyle::PM_FocusFrameVMargin));
	painter->drawText(textRectangle, Qt::AlignHCenter | Qt::TextWordWrap, index.data(Qt::DisplayRole).toString());
}


ModuleIconItem::ModuleIconItem( QListWidget* parent, const KCModuleInfo& module)
	: QListWidgetItem(SmallIcon( module.icon(), IMAGE_SIZE ), module.moduleName(), parent),
	currentState(K3Icon::DefaultState), imageName(module.icon())
{
	modules.append(module);
	setSize();
}

ModuleIconItem::ModuleIconItem( QListWidget* parent, const QString &text,
		const QString &_imageName )
	: QListWidgetItem( SmallIcon( _imageName, IMAGE_SIZE ), text, parent ),
	currentState(K3Icon::DefaultState), imageName(_imageName)
{
	setSize();
}

void ModuleIconItem::loadIcon( bool enabled )
{
	int newState = enabled ? K3Icon::DefaultState : K3Icon::DisabledState;
	if( newState == currentState )
		return;

	currentState = newState;
	setIcon( DesktopIcon( imageName, IMAGE_SIZE , currentState ) );
}

void ModuleIconItem::setSize()
{
	QStyle *style = listWidget()->style();
	QFontMetrics fm(font());
	const QRect &rect = fm.boundingRect(0, 0, ICON_WIDTH, INT_MAX, Qt::TextWordWrap, text());
	setData(Qt::SizeHintRole, QSize(ICON_WIDTH, IMAGE_SIZE + style->pixelMetric(QStyle::PM_FocusFrameVMargin) + rect.height()));
}

