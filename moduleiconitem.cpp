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

ModuleIconItemDelegate::ModuleIconItemDelegate(QObject *parent) : QItemDelegate(parent)
{
}

void ModuleIconItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	painter->save();
	painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	
	QStyle *style;
	bool selected = option.state & QStyle::State_Selected || option.state & QStyle::State_HasFocus;
	if (const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option)) {
		style = v3->widget->style();
		if (!v3->widget->hasFocus()) selected = false;
	} else {
		style = QApplication::style();
	}

	if (selected) {
		painter->fillPath(roundedRectangle(option.rect, 10), option.palette.brush(QPalette::Highlight));
		painter->setPen(option.palette.color(QPalette::HighlightedText));
	}

	if (!selected && (option.state & QStyle::State_MouseOver)) {
		QColor hover = option.palette.color(QPalette::Highlight);
		hover.setAlpha(88);
		painter->fillPath(roundedRectangle(option.rect, 10), hover);
	}

	if( index.data( Qt::UserRole ).toInt() == KIconLoader::DisabledState ) {
		painter->setPen( option.palette.color( QPalette::Disabled, QPalette::Text ) );
	}

	const QSize &decorationSize = option.decorationSize;
	QIcon::Mode iconMode = QIcon::Normal;
	if (selected) iconMode = QIcon::Selected;
	const QPixmap &pixmap = qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).pixmap(option.decorationSize, iconMode);
	int iconX = option.rect.left() + (option.rect.width() - decorationSize.width()) / 2;
	painter->drawPixmap(iconX, option.rect.top() + style->pixelMetric(QStyle::PM_FocusFrameVMargin), decorationSize.width(), decorationSize.height(), pixmap);
	
	QRect textRectangle = option.rect;
	textRectangle.setTop(textRectangle.top() + decorationSize.height() + style->pixelMetric(QStyle::PM_FocusFrameVMargin));
	painter->drawText(textRectangle, Qt::AlignHCenter | Qt::TextWordWrap, index.data(Qt::DisplayRole).toString());
	painter->restore();
}

// Method taken from KFileItemDelegate. Check whether it has been moved to
// kdefx/kdrawutil.cpp as the comment says on Fredrik's code. If so, remove
// this code (duplication), and use the library one.
QPainterPath ModuleIconItemDelegate::roundedRectangle(const QRectF &rect, qreal radius) const
{
	QPainterPath path(QPointF(rect.left(), rect.top() + radius));
	path.quadTo(rect.left(), rect.top(), rect.left() + radius, rect.top());         // Top left corner
	path.lineTo(rect.right() - radius, rect.top());                                 // Top side
	path.quadTo(rect.right(), rect.top(), rect.right(), rect.top() + radius);       // Top right corner
	path.lineTo(rect.right(), rect.bottom() - radius);                              // Right side
	path.quadTo(rect.right(), rect.bottom(), rect.right() - radius, rect.bottom()); // Bottom right corner
	path.lineTo(rect.left() + radius, rect.bottom());                               // Bottom side
	path.quadTo(rect.left(), rect.bottom(), rect.left(), rect.bottom() - radius);   // Bottom left corner
	path.closeSubpath();

	return path;
}

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
