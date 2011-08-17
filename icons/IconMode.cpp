/**************************************************************************
 * Copyright (C) 2009 by Ben Cooksley <bcooksley@kde.org>                 *
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
#include "CategoryDrawer.h"
#include "CategorizedView.h"

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
#include <KFileItemDelegate>

K_PLUGIN_FACTORY( IconModeFactory, registerPlugin<IconMode>(); )
K_EXPORT_PLUGIN( IconModeFactory( "icon_mode" ) )

class IconMode::Private {
public:
    Private() : categoryDrawer( 0 ),  categoryView( 0 ), moduleView( 0 ) {}
    virtual ~Private() {
        delete categoryDrawer;
        delete aboutIcon;
    }

    KCategoryDrawer * categoryDrawer;
    KCategorizedView * categoryView;
    QStackedWidget * mainWidget;
    MenuProxyModel * proxyModel;
    KAboutData * aboutIcon;
    ModuleView * moduleView;
    KAction * backAction;
};

IconMode::IconMode( QObject *parent, const QVariantList& )
    : BaseMode( parent )
    , d( new Private() )
{
    d->aboutIcon = new KAboutData( "IconView", 0, ki18n( "Icon View" ),
                                 "1.0", ki18n( "Provides a categorized icons view of control modules." ),
                                 KAboutData::License_GPL, ki18n( "(c) 2009, Ben Cooksley" ) );
    d->aboutIcon->addAuthor( ki18n( "Ben Cooksley" ), ki18n( "Author" ), "bcooksley@kde.org" );
    d->aboutIcon->addAuthor( ki18n( "Mathias Soeken" ), ki18n( "Developer" ), "msoeken@informatik.uni-bremen.de" );
    d->aboutIcon->setProgramIconName( "view-list-icons" );

    d->backAction = KStandardAction::back( this, SLOT(backToOverview()), this );
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
    if( !d->categoryView ) {
        initWidget();
    }
    return d->mainWidget;
}

QList<QAbstractItemView*> IconMode::views() const
{
    QList<QAbstractItemView*> list;
    list.append( d->categoryView );
    return list;
}

void IconMode::initEvent()
{
    MenuModel * model = new MenuModel( rootItem(), this );
    foreach( MenuItem * child, rootItem()->children() ) {
        model->addException( child );
    }

    d->proxyModel = new MenuProxyModel( this );
    d->proxyModel->setCategorizedModel( true );
    d->proxyModel->setSourceModel( model );
    d->proxyModel->sort( 0 );

    d->mainWidget = new QStackedWidget();
    d->moduleView = new ModuleView( d->mainWidget );
    connect( d->moduleView, SIGNAL(moduleChanged(bool)), this, SLOT(moduleLoaded()) );
    connect( d->moduleView, SIGNAL(closeRequest()), this, SLOT(backToOverview()) );
    d->categoryView = 0;
}

void IconMode::searchChanged( const QString& text )
{
    d->proxyModel->setFilterRegExp( text );
}

void IconMode::changeModule( const QModelIndex& activeModule )
{
    d->moduleView->closeModules();
    d->mainWidget->setCurrentWidget( d->moduleView );
    d->moduleView->loadModule( activeModule );
}

void IconMode::moduleLoaded()
{
    d->backAction->setEnabled( true );
    emit changeToolBarItems(BaseMode::NoItems);
}

void IconMode::backToOverview()
{
    if( d->moduleView->resolveChanges() ) {
        d->mainWidget->setCurrentWidget( d->categoryView );
        d->moduleView->closeModules();
        d->backAction->setEnabled( false );
        emit changeToolBarItems( BaseMode::Search | BaseMode::Configure | BaseMode::Quit );
        emit viewChanged( false );
    }
}

void IconMode::initWidget()
{
    // Create the widget
    d->categoryDrawer = new CategoryDrawer();
    d->categoryView = new CategorizedView( d->mainWidget );

    d->categoryView->setSelectionMode( QAbstractItemView::SingleSelection );
    d->categoryView->setSpacing( KDialog::spacingHint() );
    d->categoryView->setCategoryDrawer( d->categoryDrawer );
    d->categoryView->setViewMode( QListView::IconMode );
    d->categoryView->setMouseTracking( true );
    d->categoryView->viewport()->setAttribute( Qt::WA_Hover );

    KFileItemDelegate *delegate = new KFileItemDelegate( d->categoryView );
    delegate->setWrapMode( QTextOption::WordWrap );
    d->categoryView->setItemDelegate( delegate );

    d->categoryView->setFrameShape( QFrame::NoFrame );
    d->categoryView->setModel( d->proxyModel );
    connect( d->categoryView, SIGNAL(activated(QModelIndex)),
             this, SLOT(changeModule(QModelIndex)) );

    d->mainWidget->addWidget( d->categoryView );
    d->mainWidget->addWidget( d->moduleView );
    d->mainWidget->setCurrentWidget( d->categoryView );
}

void IconMode::leaveModuleView()
{
    d->moduleView->closeModules(); // We have to force it here
    backToOverview();
}

void IconMode::giveFocus()
{
    d->categoryView->setFocus();
}

#include "IconMode.moc"
