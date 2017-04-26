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
#include "ToolTips/tooltipmanager.h"

#include <QHBoxLayout>

#include <QAction>
#include <KAboutData>
#include <KStandardAction>
#include <KLocalizedString>
#include <KIconLoader>
#include <KServiceTypeTrader>
#include <KXmlGuiWindow>
#include <KActionCollection>
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KDeclarative/KDeclarative>
#include <QStandardItemModel>
#include <QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QDebug>

K_PLUGIN_FACTORY( SidebarModeFactory, registerPlugin<SidebarMode>(); )

class SidebarMode::Private {
public:
    Private()
      : quickWidget( nullptr ),
        moduleView( nullptr ),
        collection( nullptr ),
        activeCategory( -1 ),
        activeSubCategory( -1 )
    {}

    virtual ~Private() {
        delete aboutIcon;
    }

    ToolTipManager *toolTipManager;
    QQuickWidget * quickWidget;
    KPackage::Package package;
    QStandardItemModel * subCategoryModel;
    QWidget * mainWidget;
    QWidget * placeHolderWidget;
    QHBoxLayout * mainLayout;
    KDeclarative::KDeclarative kdeclarative;
    MenuProxyModel * proxyModel;
    KAboutData * aboutIcon;
    ModuleView * moduleView;
    KActionCollection *collection;
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
    d->proxyModel->setFilterHighlightsEntries( false );
    connect( d->proxyModel, &MenuProxyModel::filterRegExpChanged, this, &SidebarMode::activeCategoryChanged );

    d->subCategoryModel = new QStandardItemModel( this );
    d->mainWidget = new QWidget();
    d->mainWidget->installEventFilter(this);
    d->mainLayout = new QHBoxLayout(d->mainWidget);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->moduleView = new ModuleView( d->mainWidget );
    connect( d->moduleView, &ModuleView::moduleChanged, this, &SidebarMode::moduleLoaded );
    connect( d->moduleView, &ModuleView::closeRequest, this, &SidebarMode::leaveModuleView );
    d->quickWidget = 0;
    moduleView()->setFaceType(KPageView::Plain);
}

void SidebarMode::triggerGlobalAction(const QString &name)
{
    if (!d->collection) {
        return;
    }

    QAction *action = d->collection->action(name);
    if (action) {
        action->trigger();
    }
}

void SidebarMode::requestToolTip(int index, const QRectF &rect)
{
    d->toolTipManager->requestToolTip(d->proxyModel->index(index, 0), rect.toRect());
}

void SidebarMode::hideToolTip()
{
    d->toolTipManager->hideToolTip();
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
    d->placeHolderWidget->hide();
    d->moduleView->show();
    emit changeToolBarItems(BaseMode::NoItems);
}

int SidebarMode::activeCategory() const
{
    return d->proxyModel->mapFromSource(d->proxyModel->sourceModel()->index(d->activeCategory, 0)).row();
}

void SidebarMode::setActiveCategory(int cat)
{
    if (d->activeCategory == cat) {
        return;
    }

    const QModelIndex idx = d->proxyModel->index(cat, 0);
    d->activeCategory = d->proxyModel->mapToSource(idx).row();
    changeModule(idx);
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

int SidebarMode::width() const
{
    return d->mainWidget->width();
}

void SidebarMode::initWidget()
{
    // Create the widgets

    if (!KMainWindow::memberList().isEmpty()) {
        KXmlGuiWindow *mainWindow = qobject_cast<KXmlGuiWindow *>(KMainWindow::memberList().first());
        if (mainWindow) {
            d->collection = mainWindow->actionCollection();
        }
    }

    d->quickWidget = new QQuickWidget(d->mainWidget);
    d->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    d->quickWidget->engine()->rootContext()->setContextProperty("systemsettings", this);
    d->package = KPackage::PackageLoader::self()->loadPackage("KPackage/GenericQML");
    d->package.setPath(QStringLiteral("org.kde.systemsettings.sidebar"));

    d->kdeclarative.setDeclarativeEngine(d->quickWidget->engine());
    d->kdeclarative.setupBindings();

    d->quickWidget->setSource(d->package.filePath("mainscript"));

    const int rootImplicitWidth = d->quickWidget->rootObject()->property("implicitWidth").toInt();
    if (rootImplicitWidth != 0) {
        d->quickWidget->setFixedWidth(rootImplicitWidth);
    } else {
        d->quickWidget->setFixedWidth(240);
    }
    connect(d->quickWidget->rootObject(), &QQuickItem::implicitWidthChanged,
            this, [this]() {
                const int rootImplicitWidth = d->quickWidget->rootObject()->property("implicitWidth").toInt();
                if (rootImplicitWidth != 0) {
                    d->quickWidget->setFixedWidth(rootImplicitWidth);
                } else {
                    d->quickWidget->setFixedWidth(240);
                }
            });

    d->quickWidget->installEventFilter(this);

    d->toolTipManager = new ToolTipManager(d->proxyModel, d->quickWidget);

    d->placeHolderWidget = new QWidget(d->mainWidget);
    QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect(d->placeHolderWidget);
    opacity->setOpacity(0.5);
    d->placeHolderWidget->setGraphicsEffect(opacity);
    QGridLayout *placeHolderLayout = new QGridLayout(d->placeHolderWidget);
    QLabel *pictureLabel = new QLabel(d->placeHolderWidget);
    pictureLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    pictureLabel->setPixmap(QIcon::fromTheme(QStringLiteral("systemsettings")).pixmap(KIconLoader::SizeEnormous, KIconLoader::SizeEnormous));
    QLabel *introLabel = new QLabel(d->placeHolderWidget);
    introLabel->setText(i18n("Select an item from the list to see the available options"));
    placeHolderLayout->addItem(new QSpacerItem(-1,-1, QSizePolicy::Minimum, QSizePolicy::Expanding), 0, 0);
    placeHolderLayout->addWidget(pictureLabel, 1, 0, Qt::AlignCenter);
    placeHolderLayout->addWidget(introLabel, 2, 0, Qt::AlignCenter);
    placeHolderLayout->addItem(new QSpacerItem(-1,-1, QSizePolicy::Minimum, QSizePolicy::Expanding), 3, 0);

    d->mainLayout->addWidget( d->quickWidget );
    d->moduleView->hide();
    d->mainLayout->addWidget( d->moduleView );
    d->mainLayout->addWidget( d->placeHolderWidget );
    emit changeToolBarItems(BaseMode::NoItems);
}

bool SidebarMode::eventFilter(QObject* watched, QEvent* event)
{
    //TODO: patch Qt
    if (watched == d->quickWidget && event->type() == QEvent::Leave) {
        QCoreApplication::sendEvent(d->quickWidget->quickWindow(), event);
    } else if (watched == d->mainWidget && event->type() == QEvent::Resize) {
        emit widthChanged();
    }
    return BaseMode::eventFilter(watched, event);
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
