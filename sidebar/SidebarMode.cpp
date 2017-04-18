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

#include "SidebarMode.h"
#include "CategoryDrawer.h"
#include "CategorizedView.h"

#include "MenuItem.h"
#include "MenuModel.h"
#include "ModuleView.h"
#include "MenuProxyModel.h"
#include "BaseData.h"
#include "SidebarDelegate.h"

#include <QHBoxLayout>

#include <QAction>
#include <KAboutData>
#include <KStandardAction>
#include <KLocalizedString>
#include <KIconLoader>
#include <KLineEdit>
#include <KServiceTypeTrader>
#include <KXmlGuiWindow>
#include <KActionCollection>
#include <QMenu>
#include <QDebug>

K_PLUGIN_FACTORY( SidebarModeFactory, registerPlugin<SidebarMode>(); )

class SidebarMode::Private {
public:
    Private() : categoryDrawer( 0 ),  categoryView( 0 ), moduleView( 0 ) {}
    virtual ~Private() {
        delete aboutIcon;
    }

    KLineEdit * searchText;
    KCategoryDrawer * categoryDrawer;
    KCategorizedView * categoryView;
    QWidget * mainWidget;
    QToolButton * menuButton;
    QHBoxLayout * mainLayout;
    MenuProxyModel * proxyModel;
    KAboutData * aboutIcon;
    ModuleView * moduleView;
};

SidebarMode::SidebarMode( QObject *parent, const QVariantList& )
    : BaseMode( parent )
    , d( new Private() )
{
    d->aboutIcon = new KAboutData( "SidebarView", i18n( "Sidebar View" ),
                                 "1.0", i18n( "Provides a categorized sidebar for control modules." ),
                                 KAboutLicense::GPL, i18n( "(c) 2017, Marco Martin" ) );
    d->aboutIcon->addAuthor( i18n( "Marco Martin" ), i18n( "Author" ), "mart@kde.org" );
    d->aboutIcon->addAuthor( i18n( "Ben Cooksley" ), i18n( "Author" ), "bcooksley@kde.org" );
    d->aboutIcon->addAuthor( i18n( "Mathias Soeken" ), i18n( "Developer" ), "msoeken@informatik.uni-bremen.de" );
    d->aboutIcon->setProgramIconName( "view-sidetree" );
}

SidebarMode::~SidebarMode()
{
    delete d;
}

KAboutData * SidebarMode::aboutData()
{
    return d->aboutIcon;
}

ModuleView * SidebarMode::moduleView() const
{
    return d->moduleView;
}

QWidget * SidebarMode::mainWidget()
{
    if( !d->categoryView ) {
        initWidget();
    }
    return d->mainWidget;
}

QList<QAbstractItemView*> SidebarMode::views() const
{
    QList<QAbstractItemView*> list;
    list.append( d->categoryView );
    return list;
}

void SidebarMode::initEvent()
{
    MenuModel * model = new MenuModel( rootItem(), this );
    foreach( MenuItem * child, rootItem()->children() ) {
        model->addException( child );
    }

    d->proxyModel = new MenuProxyModel( this );
    d->proxyModel->setCategorizedModel( true );
    d->proxyModel->setSourceModel( model );
    d->proxyModel->sort( 0 );

    d->mainWidget = new QWidget();
    d->mainLayout = new QHBoxLayout(d->mainWidget);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->moduleView = new ModuleView( d->mainWidget );
    connect( d->moduleView, &ModuleView::moduleChanged, this, &SidebarMode::moduleLoaded );
    connect( d->moduleView, &ModuleView::closeRequest, this, &SidebarMode::leaveModuleView );
    d->categoryView = 0;
}

void SidebarMode::searchChanged( const QString& text )
{
    d->proxyModel->setFilterRegExp( text );
    if ( d->categoryView ) {
        QAbstractItemModel *model = d->categoryView->model();
        const int column = d->categoryView->modelColumn();
        const QModelIndex root = d->categoryView->rootIndex();
        for ( int i = 0; i < model->rowCount(); ++i ) {
            const QModelIndex index = model->index( i, column, root );
            if ( model->flags( index ) & Qt::ItemIsEnabled ) {
                d->categoryView->scrollTo( index );
                break;
            }
        }
    }
}

void SidebarMode::changeModule( const QModelIndex& activeModule )
{
    d->moduleView->closeModules();
    d->moduleView->loadModule( activeModule );
}

void SidebarMode::moduleLoaded()
{
    emit changeToolBarItems(BaseMode::NoItems);
}

void SidebarMode::initWidget()
{
    // Create the widgets
    QWidget *sidebar = new QWidget(d->mainWidget);
    sidebar->setBackgroundRole(QPalette::Base);
    sidebar->setFixedWidth(250);
    sidebar->setAutoFillBackground(true);
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setSpacing(0);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);

    // Initialise search
    d->searchText = new KLineEdit( sidebar );
    d->searchText->setClearButtonShown( true );
    d->searchText->setPlaceholderText( i18nc( "Search through a list of control modules", "Search" ) );
    d->searchText->setCompletionMode( KCompletion::CompletionPopup );
    QHBoxLayout *topLayout = new QHBoxLayout(sidebar);
    topLayout->setContentsMargins(0, 4, 4, 4);
    d->menuButton = new QToolButton(sidebar);
    d->menuButton->setAutoRaise(true);
    d->menuButton->setIcon( QIcon::fromTheme("application-menu") );
    if (!KMainWindow::memberList().isEmpty()) {
        KXmlGuiWindow *mainWindow = qobject_cast<KXmlGuiWindow *>(KMainWindow::memberList().first());
        if (mainWindow) {
            KActionCollection *collection = mainWindow->actionCollection();
            QMenu *menu = new QMenu(d->menuButton);
            d->menuButton->setPopupMode(QToolButton::InstantPopup);
            d->menuButton->setMenu(menu);
            menu->addAction( collection->action("configure") );
            menu->addAction( collection->action("help_contents") );
            menu->addAction( collection->action("help_about_app") );
            menu->addAction( collection->action("help_about_kde") );
        }
    }
    topLayout->addWidget( d->menuButton );
    topLayout->addWidget( d->searchText );
    sidebarLayout->addItem( topLayout );

    // Prepare the Base Data
    MenuItem *rootModule = new MenuItem( true, 0 );
    initMenuList(rootModule);
    BaseData::instance()->setMenuItem( rootModule );
    connect(d->searchText, &KLineEdit::textChanged, this, &SidebarMode::searchChanged);
    d->searchText->completionObject()->setIgnoreCase( true );
    d->searchText->completionObject()->setItems( BaseData::instance()->menuItem()->keywords() );

    d->categoryView = new CategorizedView( sidebar );
    sidebarLayout->addWidget( d->categoryView );
    d->categoryDrawer = new CategoryDrawer(d->categoryView);

    d->categoryView->setSelectionMode( QAbstractItemView::SingleSelection );
    d->categoryView->setCategoryDrawer( d->categoryDrawer );
    d->categoryView->setCategorySpacing(0);
    d->categoryView->setIconSize(QSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium));
    d->categoryView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    d->categoryView->setViewMode( QListView::ListMode );
    d->categoryView->setMouseTracking( true );
    d->categoryView->viewport()->setAttribute( Qt::WA_Hover );

    SidebarDelegate *delegate = new SidebarDelegate( d->categoryView );
    d->categoryView->setItemDelegate( delegate );

    d->categoryView->setFrameShape( QFrame::NoFrame );
    d->categoryView->setModel( d->proxyModel );
    connect( d->categoryView, &QAbstractItemView::activated,
             this, &SidebarMode::changeModule );


    d->mainLayout->addWidget( sidebar );
    d->mainLayout->addWidget( d->moduleView );
    emit changeToolBarItems(BaseMode::NoItems);
    d->searchText->setFocus(Qt::OtherFocusReason);
}

void SidebarMode::initMenuList(MenuItem * parent)
{
    KService::List categories = KServiceTypeTrader::self()->query("SystemSettingsCategory");
    KService::List modules = KServiceTypeTrader::self()->query("KCModule", "[X-KDE-System-Settings-Parent-Category] != ''");

    // look for any categories inside this level, and recurse into them
    for (int i = 0; i < categories.size(); ++i) {
        const KService::Ptr entry = categories.at(i);
        const QString parentCategory = entry->property("X-KDE-System-Settings-Parent-Category").toString();
        const QString parentCategory2 = entry->property("X-KDE-System-Settings-Parent-Category-V2").toString();
        if ( parentCategory == parent->category() ||
             // V2 entries must not be empty if they want to become a proper category.
             ( !parentCategory2.isEmpty() && parentCategory2 == parent->category() ) ) {
            MenuItem * menuItem = new MenuItem(true, parent);
            menuItem->setService( entry );
            if( menuItem->category() == "lost-and-found" ) {
                //lostFound = menuItem;
                continue;
            }
            initMenuList( menuItem );
        }
    }

    KService::List removeList;

    // scan for any modules at this level and add them
    for (int i = 0; i < modules.size(); ++i) {
        const KService::Ptr entry = modules.at(i);
        const QString category = entry->property("X-KDE-System-Settings-Parent-Category").toString();
        const QString category2 = entry->property("X-KDE-System-Settings-Parent-Category-V2").toString();
        if( !parent->category().isEmpty() && (category == parent->category() || category2 == parent->category()) ) {
            // Add the module info to the menu
            MenuItem * infoItem = new MenuItem(false, parent);
            infoItem->setService( entry );
            removeList.append( modules.at(i) );
        }
    }

    for (int i = 0; i < removeList.size(); ++i) {
        modules.removeOne( removeList.at(i) );
    }
    
    parent->sortChildrenByWeight();
}

void SidebarMode::leaveModuleView()
{
    d->moduleView->closeModules(); // We have to force it here
}

void SidebarMode::giveFocus()
{
    d->categoryView->setFocus();
}

#include "SidebarMode.moc"
