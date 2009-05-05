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
        model->addException( childItem );
        MenuProxyModel *proxyModel = new MenuProxyModel( this );
        proxyModel->setCategorizedModel( true );
        proxyModel->setSourceModel( model );
        proxyModel->sort( 0 );
        d->proxyMap.insert( proxyModel, childItem->service()->name() );
        d->proxyList << proxyModel;
    }

    d->mainWidget = new QStackedWidget();
    d->moduleView = new ModuleView( d->mainWidget );
    connect( d->moduleView, SIGNAL( moduleSwitched() ), this, SLOT( moduleLoaded() ) );
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
    if( d->moduleView->closeModules() ) {
        d->iconWidget->show();
        d->mainWidget->setCurrentWidget( d->iconWidget );
        d->backAction->setEnabled( false );
        emit changeToolBarItems( BaseMode::Search | BaseMode::Configure );
        emit viewChanged();
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
        KCategoryDrawer *drawer = new KCategoryDrawer();
        d->mCategoryDrawers << drawer;

        KCategorizedView *tv = new KCategorizedView( d->iconWidget );
        tv->setSelectionMode( QAbstractItemView::SingleSelection );
        tv->setSpacing( KDialog::spacingHint() );
        tv->setCategoryDrawer( drawer );
        tv->setViewMode( QListView::IconMode );
        tv->setMouseTracking( true );
        tv->viewport()->setAttribute( Qt::WA_Hover );
        tv->setItemDelegate( new KFileItemDelegate( tv ) );
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

#include "IconMode.moc"
