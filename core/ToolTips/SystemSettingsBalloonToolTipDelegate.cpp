/***************************************************************************
 *   Copyright (C) 2008 by Simon St James <kdedevel@etotheipiplusone.com>  *
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

#include "SystemSettingsBalloonToolTipDelegate.h"

#include "SystemSettingsToolTipItem.h"

#include <QList>
#include <QPair>
#include <QTextDocument>

#include <KApplication>
#include <KColorScheme>

QPoint currentPosition;
QRect docRect;

SystemSettingsBalloonToolTipDelegate::SystemSettingsBalloonToolTipDelegate()
{
}

SystemSettingsBalloonToolTipDelegate::~SystemSettingsBalloonToolTipDelegate()
{
}

QSize SystemSettingsBalloonToolTipDelegate::sizeHint(const KStyleOptionToolTip& option, const KToolTipItem& item) const
{
    Q_UNUSED( option );

    QTextDocument doc;
    doc.setHtml(item.text());
    const QIcon icon = item.icon();

    const QSize iconSize = icon.isNull() ? QSize(0, 0) : icon.actualSize( QSize( PREVIEW_WIDTH, PREVIEW_HEIGHT ) );
    const QSize docSize = doc.size().toSize();
    QSize contentSize = iconSize + docSize;

    // assure that the content height is large enough for the icon and the document
    contentSize.setHeight( qMax( iconSize.height(), (int)doc.size().height() ) );

    const SystemSettingsToolTipItem* controlItem = static_cast<const SystemSettingsToolTipItem*>( &item );
    if ( controlItem->lines().count() ) {
        contentSize += QSize( 0, ( controlItem->lines().count() * SUB_PREVIEW_HEIGHT ) + 4 );

        for ( int i = 0; i < controlItem->lines().count(); ++i ) {
            doc.setHtml( controlItem->lines()[i].second );
            int newWidth = SUB_PREVIEW_WIDTH + doc.size().toSize().width();
            contentSize.setWidth( qMax( contentSize.width(), newWidth ) );
        }
    }
    return contentSize + QSize(Border * 3, Border * 2);
}

void SystemSettingsBalloonToolTipDelegate::paint(QPainter* painter, const KStyleOptionToolTip& option, const KToolTipItem& item) const
{
    KColorScheme paintColors( QPalette::Normal, KColorScheme::Tooltip );
    QColor toColor = paintColors.background().color();
    QColor fromColor = KColorScheme::shade(toColor, KColorScheme::LightShade, 0.2);

    QString itemTextTemplate = "<font color=\"" + paintColors.foreground().color().name() + "\">%1</font>";

    /// HANDLE BACKGROUND
    QPainterPath path = createPath(option);
    if (haveAlphaChannel()) {
        painter->setRenderHint(QPainter::Antialiasing);
        painter->translate(.5, .5);
        toColor.setAlpha(220);
        fromColor.setAlpha(220);
    }

    QLinearGradient gradient(option.rect.topLeft(), option.rect.bottomLeft());
    gradient.setColorAt(0.0, fromColor);
    gradient.setColorAt(1.0, toColor);
    painter->setPen(Qt::NoPen);
    painter->setBrush(gradient);

    painter->drawPath(path);
    /// END BACKGROUND

    // Initialise constants
    const QSize iconSize = QSize( PREVIEW_WIDTH, PREVIEW_HEIGHT );
    const QSize subIconSize = QSize( SUB_PREVIEW_WIDTH, SUB_PREVIEW_HEIGHT );

    // Set initial configuration
    currentPosition = QPoint( Border, Border );
    docRect = QRect( QPoint(0, 0), sizeHint( option, item ) );

    // Paint primary item
    paintItem( painter, item.icon(), iconSize, itemTextTemplate.arg( item.text() ) );

    const SystemSettingsToolTipItem * controlItem = static_cast<const SystemSettingsToolTipItem*>( &item );
    if ( controlItem->lines().count() ) {
        currentPosition.setY( currentPosition.y() + 2 );
        painter->setBrush( paintColors.foreground() );
        painter->drawLine( docRect.left() + Border, currentPosition.y(), docRect.right() - Border, currentPosition.y() );

        for ( int i = 0; i < controlItem->lines().count(); ++i ) {
            QString subItemText = itemTextTemplate.arg( controlItem->lines()[i].second );
            paintItem( painter, controlItem->lines()[i].first, subIconSize, subItemText );
        }
    }

}

void SystemSettingsBalloonToolTipDelegate::paintItem( QPainter * painter, QIcon icon, QSize itemIconSize, QString text )
{
    QTextDocument itemDoc;
    itemDoc.setHtml( text );

    // Get sizes
    const QSize itemTextSize = itemDoc.size().toSize();
    int itemHeight = itemIconSize.height();

    if( itemIconSize.height() == PREVIEW_HEIGHT ) {
        itemHeight = qMax( itemIconSize.height(), itemTextSize.height() );
    }

    // Paint icon if we have one
    if( !icon.isNull() ) {
        const QRect preIconRect( currentPosition, itemIconSize );
        const QRect iconRect = QStyle::visualRect( kapp->layoutDirection(), docRect, preIconRect );
        painter->drawPixmap( iconRect, icon.pixmap(itemIconSize) );
        currentPosition.setX( Border + itemIconSize.width() );
    }

    // Paint text to a temporary buffer
    QPixmap textBitmap( itemTextSize );
    textBitmap.fill( Qt::transparent );
    QPainter textPainter( &textBitmap );
    itemDoc.drawContents( &textPainter );

    // Paint text to screen
    const QRect textPosition( currentPosition, itemTextSize );
    const QRect textRect = QStyle::visualRect( kapp->layoutDirection(), docRect, textPosition );
    painter->drawPixmap( textRect, textBitmap );

    // Reset position
    currentPosition.setX( Border );
    currentPosition.setY( itemHeight + currentPosition.y() );
}

#include "SystemSettingsBalloonToolTipDelegate.moc"
