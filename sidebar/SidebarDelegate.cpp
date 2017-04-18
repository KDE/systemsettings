/**************************************************************************
 * Copyright (C) 2017 Marco Martin <mart@kde.org>                         *
 *                                                                        *
 * This program is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU General Public License            *
 * as published by the Free Software Foundation; either version 2         *
 * of the License, or (at your option) any later version.                 *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA          *
 * 02110-1301, USA.                                                       *
***************************************************************************/

#include "SidebarDelegate.h"

#include <QApplication>
#include <QPainter>

static const int s_margin = 10;

SidebarDelegate::SidebarDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

SidebarDelegate::~SidebarDelegate()
{
}


QSize SidebarDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.decorationSize.width() + s_margin * 2, option.decorationSize.height() + s_margin * 2);
}

void SidebarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    painter->save();
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // Apparently some widget styles expect this hint to not be set
    painter->setRenderHint(QPainter::Antialiasing, false);

    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    painter->setRenderHint(QPainter::Antialiasing);

    QIcon::Mode iconMode;

    if (!(opt.state & QStyle::State_Enabled)) {
        iconMode = QIcon::Disabled;
    } else if ((opt.state & QStyle::State_Selected) && (opt.state & QStyle::State_Active)) {
        iconMode = QIcon::Selected;
    } else {
        iconMode = QIcon::Normal;
    }

    QIcon::State iconState = opt.state & QStyle::State_Open ? QIcon::On : QIcon::Off;
    QPixmap icon = opt.icon.pixmap(opt.decorationSize, iconMode, iconState);

    const QRect iconRect = QStyle::alignedRect(opt.direction, Qt::AlignVCenter | Qt::AlignLeft, opt.decorationSize, opt.rect.adjusted(s_margin, s_margin, -s_margin, -s_margin));
    painter->drawPixmap(iconRect.topLeft(), icon);

    QFontMetrics fontMetrics(opt.font);
    const QString text = fontMetrics.elidedText(opt.text, Qt::ElideRight, opt.rect.width() - iconRect.width() - s_margin*3);
    //TODO: LTR
    const QRect textRect = opt.rect.adjusted(s_margin + iconRect.right(),
                         s_margin, -s_margin, -s_margin);

    painter->setFont(opt.font);
    painter->setPen(QPen(foregroundBrush(opt, index), 0));
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    painter->restore();
}

QBrush SidebarDelegate::foregroundBrush(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QPalette::ColorGroup cg = QPalette::Active;
    if (!(option.state & QStyle::State_Enabled)) {
        cg = QPalette::Disabled;
    } else if (!(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    // Always use the highlight color for selected items
    if (option.state & QStyle::State_Selected) {
        return option.palette.brush(cg, QPalette::HighlightedText);
    }

    return option.palette.brush(cg, QPalette::Text);
}

void SidebarDelegate::initStyleOption(QStyleOptionViewItem *option,
                                         const QModelIndex &index) const
{
    QVariant value = index.data(Qt::FontRole);
    if (value.isValid() && !value.isNull()) {
        option->font = qvariant_cast<QFont>(value).resolve(option->font);
        option->fontMetrics = QFontMetrics(option->font);
    }

    value = index.data(Qt::TextAlignmentRole);
    if (value.isValid() && !value.isNull())
        option->displayAlignment = Qt::Alignment(value.toInt());

    value = index.data(Qt::ForegroundRole);
    if (value.canConvert<QBrush>())
        option->palette.setBrush(QPalette::Text, qvariant_cast<QBrush>(value));

    option->index = index;
    value = index.data(Qt::CheckStateRole);
    if (value.isValid() && !value.isNull()) {
        option->features |= QStyleOptionViewItem::HasCheckIndicator;
        option->checkState = static_cast<Qt::CheckState>(value.toInt());
    }

    value = index.data(Qt::DecorationRole);
    if (value.isValid() && !value.isNull()) {
        option->features |= QStyleOptionViewItem::HasDecoration;
        switch (value.type()) {
        case QVariant::Icon: {
            option->icon = qvariant_cast<QIcon>(value);
            QIcon::Mode mode;
            if (!(option->state & QStyle::State_Enabled))
                mode = QIcon::Disabled;
            else if (option->state & QStyle::State_Selected)
                mode = QIcon::Selected;
            else
                mode = QIcon::Normal;
            QIcon::State state = option->state & QStyle::State_Open ? QIcon::On : QIcon::Off;
            QSize actualSize = option->icon.actualSize(option->decorationSize, mode, state);
            // For highdpi icons actualSize might be larger than decorationSize, which we don't want. Clamp it to decorationSize.
            option->decorationSize = QSize(qMin(option->decorationSize.width(), actualSize.width()),
                                           qMin(option->decorationSize.height(), actualSize.height()));
            break;
        }
        case QVariant::Color: {
            QPixmap pixmap(option->decorationSize);
            pixmap.fill(qvariant_cast<QColor>(value));
            option->icon = QIcon(pixmap);
            break;
        }
        case QVariant::Image: {
            QImage image = qvariant_cast<QImage>(value);
            option->icon = QIcon(QPixmap::fromImage(image));
            option->decorationSize = image.size() / image.devicePixelRatio();
            break;
        }
        case QVariant::Pixmap: {
            QPixmap pixmap = qvariant_cast<QPixmap>(value);
            option->icon = QIcon(pixmap);
            option->decorationSize = pixmap.size() / pixmap.devicePixelRatio();
            break;
        }
        default:
            break;
        }
    }

    value = index.data(Qt::DisplayRole);
    if (value.isValid() && !value.isNull()) {
        option->features |= QStyleOptionViewItem::HasDisplay;
        option->text = value.toString();
    }

    option->backgroundBrush = qvariant_cast<QBrush>(index.data(Qt::BackgroundRole));

    // disable style animations for checkboxes etc. within itemviews (QTBUG-30146)
    option->styleObject = 0;
}
