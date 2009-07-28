/**************************************************************************
 * Copyright (C) 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>     *
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

#include "IconMode.h"

#include "MenuItem.h"
#include "MenuModel.h"
#include "ModuleView.h"
#include "MenuProxyModel.h"

#include <QPainter>
#include <QApplication>
#include <QStackedWidget>

#include <KAction>
#include <KDialog>
#include <KTabWidget>
#include <KAboutData>
#include <KStandardAction>
#include <KCategoryDrawer>
#include <KCategorizedView>
#include <KFileItemDelegate>

K_PLUGIN_FACTORY( IconModeFactory, registerPlugin<IconMode>(); )
K_EXPORT_PLUGIN( IconModeFactory( "icon_mode" ) )

class IconMode::Private {
public:
  Private() : moduleView( 0 ) {}
    virtual ~Private() {
        qDeleteAll( mCategoryDrawers );
        delete aboutIcon;
    }

    QList<KCategoryDrawer*> mCategoryDrawers;
    QList<QAbstractItemView*> mViews;
    QStackedWidget * mainWidget;
    KTabWidget * iconWidget;
    QList<MenuProxyModel *> proxyList;
    QHash<MenuProxyModel *, QString> proxyMap;
    KAboutData * aboutIcon;
    ModuleView * moduleView;
    KAction * backAction;
};

class IconMode::CategoryDrawer
    : public KCategoryDrawerV2
{
public:
    CategoryDrawer();

    virtual void drawCategory(const QModelIndex &index,
                              int sortRole,
                              const QStyleOption &option,
                              QPainter *painter) const;

    virtual int categoryHeight(const QModelIndex &index, const QStyleOption &option) const;
};

IconMode::CategoryDrawer::CategoryDrawer()
{
    setLeftMargin( 7 );
    setRightMargin( 7 );
}

void IconMode::CategoryDrawer::drawCategory(const QModelIndex &index,
                                            int sortRole,
                                            const QStyleOption &option,
                                            QPainter *painter) const
{
    painter->setRenderHint(QPainter::Antialiasing);

    const QRect optRect = option.rect;
    QFont font(QApplication::font());
    font.setBold(true);
    const QFontMetrics fontMetrics = QFontMetrics(font);
    const int height = categoryHeight(index, option);

    //BEGIN: decoration gradient
    {
        QPainterPath path(optRect.bottomLeft());
        path.lineTo(QPoint(optRect.topLeft().x(), optRect.topLeft().y() - 3));
        const QPointF topLeft(optRect.topLeft());
        QRectF arc(topLeft, QSizeF(4, 4));
        path.arcTo(arc, 180, -90);
        path.lineTo(optRect.topRight());
        path.lineTo(optRect.bottomRight());
        path.lineTo(optRect.bottomLeft());

        QColor window(option.palette.window().color());
        const QColor base(option.palette.base().color());

        window.setAlphaF(0.4);

        QLinearGradient decoGradient1(optRect.topLeft(), optRect.bottomLeft());
        decoGradient1.setColorAt(0, window);
        decoGradient1.setColorAt(1, Qt::transparent);

        QLinearGradient decoGradient2(optRect.topLeft(), optRect.topRight());
        decoGradient2.setColorAt(0, Qt::transparent);
        decoGradient2.setColorAt(1, base);

        painter->fillPath(path, decoGradient1);
        painter->fillPath(path, decoGradient2);
    }
    //END: decoration gradient

    {
        QRect newOptRect(optRect);
        newOptRect.setLeft(newOptRect.left() + 1);
        newOptRect.setTop(newOptRect.top() + 1);

        //BEGIN: inner top left corner
        {
            painter->save();
            painter->setPen(option.palette.base().color());
            const QPointF topLeft(newOptRect.topLeft());
            QRectF arc(topLeft, QSizeF(4, 4));
            arc.translate(0.5, 0.5);
            painter->drawArc(arc, 1440, 1440);
            painter->restore();
        }
        //END: inner top left corner

        //BEGIN: inner left vertical line
        {
            QPoint start(newOptRect.topLeft());
            start.ry() += 3;
            QPoint verticalGradBottom(newOptRect.topLeft());
            verticalGradBottom.ry() += newOptRect.height() - 3;
            QLinearGradient gradient(start, verticalGradBottom);
            gradient.setColorAt(0, option.palette.base().color());
            gradient.setColorAt(1, Qt::transparent);
            painter->fillRect(QRect(start, QSize(1, newOptRect.height() - 3)), gradient);
        }
        //END: inner left vertical line

        //BEGIN: inner horizontal line
        {
            QPoint start(newOptRect.topLeft());
            start.rx() += 3;
            QPoint horizontalGradTop(newOptRect.topLeft());
            horizontalGradTop.rx() += newOptRect.width() - 3;
            QLinearGradient gradient(start, horizontalGradTop);
            gradient.setColorAt(0, option.palette.base().color());
            gradient.setColorAt(1, Qt::transparent);
            painter->fillRect(QRect(start, QSize(newOptRect.width() - 3, 1)), gradient);
        }
        //END: inner horizontal line
    }

    QColor outlineColor = option.palette.text().color();
    outlineColor.setAlphaF(0.35);

    //BEGIN: top left corner
    {
        painter->save();
        painter->setPen(outlineColor);
        const QPointF topLeft(optRect.topLeft());
        QRectF arc(topLeft, QSizeF(4, 4));
        arc.translate(0.5, 0.5);
        painter->drawArc(arc, 1440, 1440);
        painter->restore();
    }
    //END: top left corner

    //BEGIN: left vertical line
    {
        QPoint start(optRect.topLeft());
        start.ry() += 3;
        QPoint verticalGradBottom(optRect.topLeft());
        verticalGradBottom.ry() += optRect.height() - 3;
        QLinearGradient gradient(start, verticalGradBottom);
        gradient.setColorAt(0, outlineColor);
        gradient.setColorAt(1, option.palette.base().color());
        painter->fillRect(QRect(start, QSize(1, optRect.height() - 3)), gradient);
    }
    //END: left vertical line

    //BEGIN: horizontal line
    {
        QPoint start(optRect.topLeft());
        start.rx() += 3;
        QPoint horizontalGradTop(optRect.topLeft());
        horizontalGradTop.rx() += optRect.width() - 3;
        QLinearGradient gradient(start, horizontalGradTop);
        gradient.setColorAt(0, outlineColor);
        gradient.setColorAt(1, option.palette.base().color());
        painter->fillRect(QRect(start, QSize(optRect.width() - 3, 1)), gradient);
    }
    //END: horizontal line

    //BEGIN: draw text
    {
        const QString category = index.model()->data(index, KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();
        QRect textRect = QRect(option.rect.topLeft(), QSize(option.rect.width(), height));
        textRect.setTop(textRect.top() + 2 + 3 /* corner */);
        textRect.setLeft(textRect.left() + 2 + 3 /* corner */ + 3 /* a bit of margin */);
        painter->save();
        painter->setFont(font);
        QColor penColor(option.palette.text().color());
        penColor.setAlphaF(0.6);
        painter->setPen(penColor);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, category);
        painter->restore();
    }
    //END: draw text
}

int IconMode::CategoryDrawer::categoryHeight(const QModelIndex &index, const QStyleOption &option) const
{
    QFont font(QApplication::font());
    font.setBold(true);
    const QFontMetrics fontMetrics = QFontMetrics(font);

    return fontMetrics.height() + 2 + 12 /* vertical spacing */;
}

class IconMode::CategorizedView
    : public KCategorizedView
{
public:
    CategorizedView( QWidget *parent = 0 );

    virtual void setModel( QAbstractItemModel *model );
};

IconMode::CategorizedView::CategorizedView( QWidget *parent )
    : KCategorizedView( parent )
{
    setWordWrap( true );
}

void IconMode::CategorizedView::setModel( QAbstractItemModel *model )
{
    KCategorizedView::setModel( model );
    int maxWidth = -1;
    int maxHeight = -1;
    for ( int i = 0; i < model->rowCount(); ++i ) {
        const QModelIndex index = model->index(i, modelColumn(), rootIndex());
        const QSize size = sizeHintForIndex( index );
        maxWidth = qMax( maxWidth, size.width() );
        maxHeight = qMax( maxHeight, size.height() );
    }
    setGridSize( QSize( maxWidth, maxHeight ) );
    static_cast<KFileItemDelegate*>( itemDelegate() )->setMaximumSize( QSize( maxWidth, maxHeight ) );
}

IconMode::IconMode( QObject *parent, const QVariantList& )
    : BaseMode( parent ), d( new Private() )
{
    d->aboutIcon = new KAboutData( "IconView", 0, ki18n( "Icon View" ),
                                 "1.0", ki18n( "Provides a categorized icons view of control modules." ),
                                 KAboutData::License_GPL, ki18n( "(c) 2009, Ben Cooksley" ) );
    d->aboutIcon->addAuthor( ki18n( "Ben Cooksley" ), ki18n( "Author" ), "ben@eclipse.endoftheinternet.org" );
    d->aboutIcon->addAuthor( ki18n( "Mathias Soeken" ), ki18n( "Developer" ), "msoeken@informatik.uni-bremen.de" );
    d->aboutIcon->setProgramIconName( "view-list-icons" );

    d->backAction = KStandardAction::back( this, SLOT( backToOverview() ), this );
    d->backAction->setText( i18n( "Overview" ) );
    d->backAction->setToolTip( i18n("Keyboard Shortcut: %1", d->backAction->shortcut().primary().toString( QKeySequence::NativeText )) );
    d->backAction->setEnabled( false );
    actionsList() << d->backAction;
}

IconMode::~IconMode()
{
    delete d;
}

KAboutData * IconMode::aboutData()
{
    return d->aboutIcon;
}

ModuleView * IconMode::moduleView() const
{
    return d->moduleView;
}

QWidget * IconMode::mainWidget()
{
    if( !d->iconWidget ) {
        initWidget();
    }
    return d->mainWidget;
}

QList<QAbstractItemView*> IconMode::views() const
{
    return d->mViews;
}

void IconMode::initEvent()
{
    foreach( MenuItem *childItem, rootItem()->children() ) {
        MenuModel *model = new MenuModel( childItem, this );
        foreach( MenuItem * child, childItem->children() ) {
            model->addException( child );
        }
        MenuProxyModel *proxyModel = new MenuProxyModel( this );
        proxyModel->setCategorizedModel( true );
        proxyModel->setSourceModel( model );
        proxyModel->sort( 0 );
        d->proxyMap.insert( proxyModel, childItem->name() );
        d->proxyList << proxyModel;
    }

    d->mainWidget = new QStackedWidget();
    d->moduleView = new ModuleView( d->mainWidget );
    connect( d->moduleView, SIGNAL( moduleChanged(bool) ), this, SLOT( moduleLoaded() ) );
    connect( d->moduleView, SIGNAL( closeRequest() ), this, SLOT( backToOverview() ) );
    d->iconWidget = 0;
}

void IconMode::searchChanged( const QString& text )
{
    foreach( MenuProxyModel *proxyModel, d->proxyList ) {
        proxyModel->setFilterRegExp( text );
    }
}

void IconMode::changeModule( const QModelIndex& activeModule )
{
    MenuItem *menuItem = activeModule.model()->data( activeModule, Qt::UserRole ).value<MenuItem*>();
    d->moduleView->loadModule( menuItem );
}

void IconMode::moduleLoaded()
{
    d->iconWidget->hide();
    d->mainWidget->setCurrentWidget( d->moduleView );
    d->backAction->setEnabled( true );
    emit changeToolBarItems(BaseMode::NoItems);
}

void IconMode::backToOverview()
{
    if( d->moduleView->resolveChanges() ) {
        d->moduleView->closeModules();
        d->iconWidget->show();
        d->mainWidget->setCurrentWidget( d->iconWidget );
        d->backAction->setEnabled( false );
        emit changeToolBarItems( BaseMode::Search | BaseMode::Configure );
        emit viewChanged( false );
    }
}

void IconMode::initWidget()
{
    // Create the widget
    d->iconWidget = new KTabWidget( d->mainWidget );
#if QT_VERSION >= 0x040500
    d->iconWidget->setDocumentMode( true );
#endif
    foreach( MenuProxyModel *proxyModel, d->proxyList ) {
        KCategoryDrawer *drawer = new CategoryDrawer();
        d->mCategoryDrawers << drawer;

        KCategorizedView *tv = new CategorizedView( d->iconWidget );
        tv->setSelectionMode( QAbstractItemView::SingleSelection );
        tv->setSpacing( KDialog::spacingHint() );
        tv->setCategoryDrawer( drawer );
        tv->setViewMode( QListView::IconMode );
        tv->setMouseTracking( true );
        tv->viewport()->setAttribute( Qt::WA_Hover );
        KFileItemDelegate *delegate = new KFileItemDelegate( tv );
        delegate->setWrapMode( QTextOption::WordWrap );
        tv->setItemDelegate( delegate );
        tv->setFrameShape( QFrame::NoFrame );
        tv->setModel( proxyModel );
        d->iconWidget->addTab( tv, d->proxyMap.value( proxyModel ) );
        connect( tv, SIGNAL( activated( const QModelIndex& ) ),
                 this, SLOT( changeModule(const QModelIndex& ) ) );

        d->mViews << tv;
    }

    d->mainWidget->addWidget( d->iconWidget );
    d->mainWidget->addWidget( d->moduleView );
    d->mainWidget->setCurrentWidget( d->iconWidget );
}

void IconMode::leaveModuleView()
{
    d->moduleView->closeModules(); // We have to force it here
    backToOverview();
}

void IconMode::giveFocus()
{
    d->iconWidget->currentWidget()->setFocus();
}

#include "IconMode.moc"
