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

#include "tooltipmanager.h"

#include "MenuItem.h"

#include <QRect>
#include <QLabel>
#include <QTimer>
#include <QScrollBar>
#include <QGridLayout>
#include <QApplication>
#include <QAbstractItemView>

#include <KIconLoader>
#include <KColorScheme>
#include <KLocalizedString>
#include <KToolTipWidget>

class ToolTipManager::Private
{
public:
    Private() :
        tooltip(nullptr),
        view(nullptr),
        timer(nullptr),
        delay(300)
        { }

    KToolTipWidget *tooltip;
    QAbstractItemView* view;
    QTimer* timer;
    QModelIndex item;
    QRect itemRect;
    int delay;
};

ToolTipManager::ToolTipManager(QAbstractItemView* parent)
    : QObject(parent)
    , d(new ToolTipManager::Private)
{
    d->view = parent;
    d->tooltip = new KToolTipWidget(d->view);
    d->tooltip->setHideDelay(0);

    connect(parent, &QAbstractItemView::viewportEntered, this, &ToolTipManager::hideToolTip);
    connect(parent, &QAbstractItemView::entered, this, &ToolTipManager::requestToolTip);

    d->timer = new QTimer(this);
    d->timer->setSingleShot(true);
    connect(d->timer, &QTimer::timeout, this, &ToolTipManager::prepareToolTip);

    // When the mousewheel is used, the items don't get a hovered indication
    // (Qt-issue #200665). To assure that the tooltip still gets hidden,
    // the scrollbars are observed.
    connect(parent->horizontalScrollBar(), &QAbstractSlider::valueChanged, this, &ToolTipManager::hideToolTip);
    connect(parent->verticalScrollBar(), &QAbstractSlider::valueChanged, this, &ToolTipManager::hideToolTip);

    d->view->viewport()->installEventFilter(this);
}

ToolTipManager::~ToolTipManager()
{
    delete d;
}

bool ToolTipManager::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == d->view->viewport()) {
        switch (event->type()) {
            case QEvent::Leave:
            case QEvent::MouseButtonPress:
                hideToolTip();
                break;
            default:
                break;
        }
    }

    return QObject::eventFilter(watched, event);
}

void ToolTipManager::requestToolTip(const QModelIndex& index)
{
    // only request a tooltip for the name column and when no selection or
    // drag & drop operation is done (indicated by the left mouse button)
    if ( !(QApplication::mouseButtons() & Qt::LeftButton) ) {
        d->tooltip->hide();

        d->itemRect = d->view->visualRect(index);
        const QPoint pos = d->view->viewport()->mapToGlobal(d->itemRect.topLeft());
        d->itemRect.moveTo(pos);
        d->item = index;
        d->timer->start(d->delay);
    } else {
        hideToolTip();
    }
}

void ToolTipManager::hideToolTip()
{
    d->timer->stop();
    d->tooltip->hideLater();
}

void ToolTipManager::prepareToolTip()
{
    showToolTip( d->item );
}

void ToolTipManager::showToolTip( const QModelIndex &menuItem )
{
    if (QApplication::mouseButtons() & Qt::LeftButton) {
        return;
    }

    QWidget * tip = createTipContent( menuItem );

    d->tooltip->showBelow(d->itemRect, tip, d->view->nativeParentWidget()->windowHandle());

    connect(d->tooltip, &KToolTipWidget::hidden, tip, &QObject::deleteLater);

}

QWidget * ToolTipManager::createTipContent( QModelIndex item )
{
    const QSize dialogIconSize = QSize(IconSize(KIconLoader::Dialog), IconSize(KIconLoader::Dialog));
    const QSize toolbarIconSize = QSize(IconSize(KIconLoader::MainToolbar), IconSize(KIconLoader::MainToolbar));

    QWidget * tipContent = new QWidget();
    QGridLayout* tipLayout = new QGridLayout();
    tipLayout->setAlignment( Qt::AlignLeft );

    QLayout * primaryLine = generateToolTipLine( &item, tipContent, dialogIconSize, true );
    primaryLine->setAlignment( Qt::AlignLeft );
    tipLayout->addLayout( primaryLine, 0, 0, Qt::AlignLeft );

    for ( int done = 0; d->view->model()->rowCount( item ) > done; done = 1 + done ) {
        QModelIndex childItem = d->view->model()->index( done, 0, item );
        QLayout * subLine = generateToolTipLine( &childItem, tipContent, toolbarIconSize, false );
        subLine->setAlignment( Qt::AlignLeft );
        tipLayout->addLayout( subLine, done + 2, 0, Qt::AlignLeft );
    }

    tipLayout->setVerticalSpacing( tipContent->fontMetrics().height() / 3 );
    tipContent->setLayout( tipLayout );

    if( d->view->model()->rowCount( item ) > 0 ) {
        QFrame * separatorLine = new QFrame( tipContent );
        separatorLine->setFrameStyle( QFrame::HLine );
        tipLayout->addWidget( separatorLine, 1, 0 );
    }

    return tipContent;
}

QLayout * ToolTipManager::generateToolTipLine( QModelIndex * item, QWidget * toolTip, QSize iconSize, bool comment )
{
    // Get MenuItem
    MenuItem * menuItem = d->view->model()->data( *item, Qt::UserRole ).value<MenuItem*>();

    QString text = menuItem->name();
    if ( comment ) {
        text = QStringLiteral( "<b>%1</b>" ).arg( menuItem->name() );
    }

    // Generate text
    if ( comment ) {
        text += QStringLiteral("<br />");
        if ( !menuItem->service()->comment().isEmpty() ) {
            text += menuItem->service()->comment();
        } else {
            int childCount = d->view->model()->rowCount( *item );
            text += i18np( "Contains 1 item", "Contains %1 items", childCount );
        }
    }
    QLabel * textLabel = new QLabel( toolTip );
    textLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    textLabel->setForegroundRole(QPalette::ToolTipText);
    textLabel->setText( text );

    // Get icon
    QLabel * iconLabel = new QLabel( toolTip );
    iconLabel->setPixmap( QIcon::fromTheme(menuItem->service()->icon()).pixmap(iconSize) );
    iconLabel->setMaximumSize( iconSize );

    // Generate layout
    QHBoxLayout * layout = new QHBoxLayout();
    layout->setSpacing( textLabel->fontMetrics().height() / 3 );
    layout->setAlignment( Qt::AlignLeft );
    layout->addWidget( iconLabel, Qt::AlignLeft );
    layout->addWidget( textLabel, Qt::AlignLeft );

    return layout;
}

