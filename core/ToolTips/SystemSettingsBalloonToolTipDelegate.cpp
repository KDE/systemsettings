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

#include <KDebug>
#include <KColorScheme>

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

    const QIcon icon = item.icon();
    int x = Border;
    const int y = Border;
    if (!icon.isNull()) {
        const QSize iconSize = QSize( PREVIEW_WIDTH, PREVIEW_HEIGHT );
        const QPoint pos(x + option.rect.x(), y + option.rect.y());
        painter->drawPixmap(pos, icon.pixmap(iconSize));
        x += iconSize.width() + Border;
    }

    QTextDocument doc;
    doc.setHtml( "<font color=\"" + paintColors.foreground().color().name() + "\">" + item.text() + "</font>" );
    QPixmap bitmap(doc.size().toSize());
    bitmap.fill(Qt::transparent);
    QPainter p(&bitmap);
    doc.drawContents(&p);

    const QRect docRect(QPoint(x, y), doc.size().toSize());
    painter->drawPixmap(docRect, bitmap);

    const SystemSettingsToolTipItem* controlItem = static_cast<const SystemSettingsToolTipItem*>( &item );
    if ( controlItem->lines().count() ) {
        int ypos = Border + qMax( PREVIEW_HEIGHT, doc.size().toSize().height() ) + 2;
        painter->setBrush( paintColors.foreground() );
        painter->drawLine( Border + option.rect.x(), ypos, option.rect.right() - Border, ypos );
        ypos += 2;

        const QSize subIconSize = QSize( SUB_PREVIEW_WIDTH, SUB_PREVIEW_HEIGHT );
        for ( int i = 0; i < controlItem->lines().count(); ++i ) {
            painter->drawPixmap( QPoint( Border + option.rect.x(), ypos ), controlItem->lines()[i].first.pixmap( subIconSize ) );

            QTextDocument doc;
            doc.setHtml( "<font color=\"" + paintColors.foreground().color().name() + "\">" + controlItem->lines()[i].second + "</font>" );
            QPixmap bitmap( doc.size().toSize() );
            bitmap.fill( Qt::transparent );
            QPainter p( &bitmap );
            doc.drawContents( &p );

            const QRect docRect( QPoint( 2 * Border + option.rect.x() + subIconSize.width(), ypos ), doc.size().toSize() );
            painter->drawPixmap( docRect, bitmap );

            ypos += SUB_PREVIEW_HEIGHT;
        }
    }
}

#include "SystemSettingsBalloonToolTipDelegate.moc"
