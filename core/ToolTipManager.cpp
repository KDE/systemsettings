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

#include "ToolTipManager.h"

#include "MenuItem.h"

#include "KToolTip.h"
#include "KToolTipManager.h"
#include "SystemSettingsToolTipItem.h"
#include "SystemSettingsBalloonToolTipDelegate.h"

#include <QRect>
#include <QTimer>
#include <QToolTip>
#include <QHelpEvent>
#include <QScrollBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QAbstractItemView>

#include <KIcon>

K_GLOBAL_STATIC(SystemSettingsBalloonToolTipDelegate, g_delegate)

class ToolTipManager::Private
{
public:
    Private() :
        view(0),
        timer(0)
        { }

    QAbstractItemView* view;
    KSharedPtr<KToolTipManager> tooltipManager;
    QTimer* timer;
    QModelIndex item;
    QRect itemRect;
};

ToolTipManager::ToolTipManager(QAbstractItemView* parent) :
    QObject(parent),
    d(new ToolTipManager::Private)
{
    d->view = parent;
    d->tooltipManager = KToolTipManager::instance();
    KToolTip::setToolTipDelegate(g_delegate);

    connect(parent, SIGNAL(viewportEntered()), this, SLOT(hideToolTip()));

    d->timer = new QTimer(this);
    d->timer->setSingleShot(true);
    connect(d->timer, SIGNAL(timeout()), this, SLOT(prepareToolTip()));

    // When the mousewheel is used, the items don't get a hovered indication
    // (Qt-issue #200665). To assure that the tooltip still gets hidden,
    // the scrollbars are observed.
    connect(parent->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(hideToolTip()));
    connect(parent->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(hideToolTip()));

    d->view->viewport()->installEventFilter(this);
}

ToolTipManager::~ToolTipManager()
{
    delete d;
}

bool ToolTipManager::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == d->view->viewport()) {
        if (event->type() == QEvent::Leave) {
            hideToolTip();
        }
        if (event->type() == QEvent::ToolTip) {
            QHelpEvent * helpEvent = static_cast<QHelpEvent *>(event);
            QModelIndex index = d->view->indexAt(helpEvent->pos());
            if( index != QModelIndex() ) {
                d->itemRect = d->view->visualRect(index);
                const QPoint pos = d->view->viewport()->mapToGlobal(d->itemRect.topLeft());
                d->itemRect.moveTo(pos);
                d->item = index;
                d->timer->start(50);
            }
            return ( index != QModelIndex() );
        }
    }

    return QObject::eventFilter(watched, event);
}

void ToolTipManager::hideToolTip()
{
    d->timer->stop();
    KToolTip::hideTip();
}

void ToolTipManager::prepareToolTip()
{
    QAbstractItemModel * itemModel = d->view->model();
    MenuItem * m_Menu = itemModel->data( d->item, Qt::UserRole ).value<MenuItem*>();
    const QString text = generateToolTipContent( d->item, m_Menu );
    SystemSettingsToolTipItem* toolTip = new SystemSettingsToolTipItem(KIcon( m_Menu->service()->icon() ), text);

    for ( int done = 0; itemModel->rowCount( d->item ) > done; done = 1 + done ) {
        QModelIndex childIndex = itemModel->index( done, 0, d->item );
        MenuItem * child = itemModel->data( childIndex, Qt::UserRole ).value<MenuItem*>();
        const QString text = QString( "%1<br />" ).arg( child->name() );
        toolTip->addLine( KIcon( child->service()->icon() ), text );
    }

    showToolTip(toolTip);
}

QString ToolTipManager::generateToolTipContent( QModelIndex index, MenuItem * item )
{
    QString text = QString( "<b>%1</b><br />%2" ).arg( item->name() );
    if ( !item->service()->comment().isEmpty() ) {
        text = text.arg( item->service()->comment() );
    } else {
        int childCount = d->view->model()->rowCount( index );
        text = text.arg( i18np( "<i>Contains 1 item</i>", "<i>Contains %1 items</i>", childCount ) );
    }
    return text;
}

void ToolTipManager::showToolTip(KToolTipItem* tip)
{
    if (QApplication::mouseButtons() & Qt::LeftButton) {
        delete tip;
        tip = 0;
        return;
    }

    KStyleOptionToolTip option;
    d->tooltipManager->initStyleOption(&option);

    QSize size = g_delegate->sizeHint(option, *tip);
    const QRect desktop = QApplication::desktop()->screenGeometry(d->itemRect.bottomRight());

    // m_itemRect defines the area of the item, where the tooltip should be
    // shown. Per default the tooltip is shown in the bottom right corner.
    // If the tooltip content exceeds the desktop borders, it must be assured that:
    // - the content is fully visible
    // - the content is not drawn inside m_itemRect
    const bool hasRoomToLeft  = (d->itemRect.left()   - size.width()  >= desktop.left());
    const bool hasRoomToRight = (d->itemRect.right()  + size.width()  <= desktop.right());
    const bool hasRoomAbove   = (d->itemRect.top()    - size.height() >= desktop.top());
    const bool hasRoomBelow   = (d->itemRect.bottom() + size.height() <= desktop.bottom());
    if ( ( !hasRoomAbove && !hasRoomBelow ) || ( !hasRoomToLeft && !hasRoomToRight ) ) {
        delete tip;
        tip = 0;
        return;
    }

    int x = 0;
    int y = 0;
    if (hasRoomBelow || hasRoomAbove) {
        x = QCursor::pos().x() + 16; // TODO: use mouse pointer width instead of the magic value of 16
        if (x + size.width() >= desktop.right()) {
            x = desktop.right() - size.width();
        }
        y = hasRoomBelow ? d->itemRect.bottom() : d->itemRect.top() - size.height();
    } else {
        Q_ASSERT(hasRoomToLeft || hasRoomToRight);
        x = hasRoomToRight ? d->itemRect.right() : d->itemRect.left() - size.width();

        // Put the tooltip at the bottom of the screen. The x-coordinate has already
        // been adjusted, so that no overlapping with m_itemRect occurs.
        y = desktop.bottom() - size.height();
    }

    KToolTip::showTip(QPoint(x, y), tip);
}

#include "ToolTipManager.moc"
