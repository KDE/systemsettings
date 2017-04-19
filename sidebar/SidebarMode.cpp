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
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <QStandardItemModel>
#include <QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <QMenu>
#include <QDebug>

K_PLUGIN_FACTORY( SidebarModeFactory, registerPlugin<SidebarMode>(); )

class SidebarMode::Private {
public:
    Private() : quickWidget( 0 ), moduleView( 0 ), activeCategory( -1 ), activeSubCategory( -1 ) {}
    virtual ~Private() {
        delete aboutIcon;
    }

    QQuickWidget * quickWidget;
    KPackage::Package package;
    QStandardItemModel * subCategoryModel;
    QWidget * mainWidget;
    QHBoxLayout * mainLayout;
    MenuProxyModel * proxyModel;
    KAboutData * aboutIcon;
    ModuleView * moduleView;
    QList<QObject *> globalActions;
    int activeCategory;
    int activeSubCategory;
};

SidebarMode::SidebarMode( QObject *parent, const QVariantList& )
    : BaseMode( parent )
    , d( new Private() )
{
    qApp->setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
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
    if( !d->quickWidget ) {
        initWidget();
    }
    return d->mainWidget;
}

QAbstractItemModel * SidebarMode::categoryModel() const
{
    return d->proxyModel;
}

QAbstractItemModel * SidebarMode::subCategoryModel() const
{
    return d->subCategoryModel;
}

QList<QObject *> SidebarMode::globalActions() const
{
    return d->globalActions;
}

QList<QAbstractItemView*> SidebarMode::views() const
{
    QList<QAbstractItemView*> list;
    //list.append( d->categoryView );
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
    d->proxyModel->setFilterHighlightsEntries( false );

    d->subCategoryModel = new QStandardItemModel( this );
    d->mainWidget = new QWidget();
    d->mainLayout = new QHBoxLayout(d->mainWidget);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->moduleView = new ModuleView( d->mainWidget );
    connect( d->moduleView, &ModuleView::moduleChanged, this, &SidebarMode::moduleLoaded );
    connect( d->moduleView, &ModuleView::closeRequest, this, &SidebarMode::leaveModuleView );
    d->quickWidget = 0;
    moduleView()->setFaceType(KPageView::Plain);
}

void SidebarMode::changeModule( const QModelIndex& activeModule )
{
    d->moduleView->closeModules();

    d->subCategoryModel->clear();
    const int subRows = d->proxyModel->rowCount(activeModule);
    if ( subRows < 2) {
        d->moduleView->loadModule( activeModule );
    } else {
        for (int i = 0; i < subRows; ++i) {
            const QModelIndex& index = d->proxyModel->index(i, 0, activeModule);
            QStandardItem *item = new QStandardItem(d->proxyModel->data(index, Qt::DecorationRole).value<QIcon>(), d->proxyModel->data(index, Qt::DisplayRole).toString());
            item->setData(index.data(Qt::UserRole), Qt::UserRole);
            d->subCategoryModel->appendRow(item);
        }
        d->moduleView->loadModule( d->proxyModel->index(0, 0, activeModule) );
    }
}

void SidebarMode::moduleLoaded()
{
    emit changeToolBarItems(BaseMode::NoItems);
}

int SidebarMode::activeCategory() const
{
    return d->activeCategory;
}

void SidebarMode::setActiveCategory(int cat)
{
    if (d->activeCategory == cat) {
        return;
    }

    d->activeCategory = cat;
    changeModule(d->proxyModel->index(cat, 0));
    d->activeSubCategory = 0;
    emit activeCategoryChanged();
    emit activeSubCategoryChanged();
}

int SidebarMode::activeSubCategory() const
{
    return d->activeSubCategory;
}

void SidebarMode::setActiveSubCategory(int cat)
{
    if (d->activeSubCategory == cat) {
        return;
    }

    d->activeSubCategory = cat;
    d->moduleView->closeModules();
    d->moduleView->loadModule( d->subCategoryModel->index(cat, 0) );
    emit activeSubCategoryChanged();
}

void SidebarMode::initWidget()
{
    // Create the widgets

    if (!KMainWindow::memberList().isEmpty()) {
        KXmlGuiWindow *mainWindow = qobject_cast<KXmlGuiWindow *>(KMainWindow::memberList().first());
        if (mainWindow) {
            KActionCollection *collection = mainWindow->actionCollection();
            d->globalActions << collection->action("configure")
                             << collection->action("help_contents")
                             << collection->action("help_about_app")
                             << collection->action("help_about_kde");
        }
    }

    d->quickWidget = new QQuickWidget(d->mainWidget);
    d->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    d->quickWidget->engine()->rootContext()->setContextProperty("systemsettings", this);
    d->package = KPackage::PackageLoader::self()->loadPackage("KPackage/GenericQML");
    d->package.setPath(QStringLiteral("org.kde.systemsettings.sidebar"));
    d->quickWidget->setSource(d->package.filePath("mainscript"));
    //FIXME
    d->quickWidget->setFixedWidth(240);

    // Prepare the Base Data
    MenuItem *rootModule = new MenuItem( true, 0 );
    initMenuList(rootModule);
    BaseData::instance()->setMenuItem( rootModule );

    d->mainLayout->addWidget( d->quickWidget );
    d->mainLayout->addWidget( d->moduleView );
    emit changeToolBarItems(BaseMode::NoItems);
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
    d->quickWidget->setFocus();
}

#include "SidebarMode.moc"
