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
#include <KCModuleInfo>
#include <KStandardAction>
#include <KLocalizedString>
#include <KIconLoader>
#include <KLocalizedContext>
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

#include <KActivities/Stats/ResultModel>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/Terms>

namespace KAStats = KActivities::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

K_PLUGIN_FACTORY( SidebarModeFactory, registerPlugin<SidebarMode>(); )

class SubcategoryModel : public QStandardItemModel
{
public:
    SubcategoryModel(QAbstractItemModel *parentModel, QObject *parent = 0)
        : QStandardItemModel(parent),
          m_parentModel(parentModel)
    {}

    void setParentIndex(const QModelIndex &activeModule)
    {
        blockSignals(true);
        //make the view receive a single signal when the new subcategory is loaded,
        //never make the view believe there are zero items if this is not the final count
        //this avoids the brief flash it had
        clear();
        const int subRows = m_parentModel->rowCount(activeModule);
        if ( subRows > 1) {
            for (int i = 0; i < subRows; ++i) {
                const QModelIndex& index = m_parentModel->index(i, 0, activeModule);
                QStandardItem *item = new QStandardItem(m_parentModel->data(index, Qt::DecorationRole).value<QIcon>(), m_parentModel->data(index, Qt::DisplayRole).toString());
                item->setData(index.data(Qt::UserRole), Qt::UserRole);
                appendRow(item);
            }
        }
        blockSignals(false);
        beginResetModel();
        endResetModel();
    }

private:
    QAbstractItemModel *m_parentModel;
};

class MostUsedModel : public QSortFilterProxyModel
{
public:
    MostUsedModel(QObject *parent = 0)
        : QSortFilterProxyModel (parent)
    {
        sort(0, Qt::DescendingOrder);
        setSortRole(ResultModel::ScoreRole);
        setDynamicSortFilter(true);
        //prepare default items
        m_defaultModel = new QStandardItemModel(this);
        QStandardItem *item = new QStandardItem();
        item->setData(QUrl(QStringLiteral("kcm:kcm_lookandfeel.desktop")), ResultModel::ResourceRole);
        m_defaultModel->appendRow(item);
        item = new QStandardItem();
        item->setData(QUrl(QStringLiteral("kcm:user_manager.desktop")), ResultModel::ResourceRole);
        m_defaultModel->appendRow(item);
        item = new QStandardItem();
        item->setData(QUrl(QStringLiteral("kcm:screenlocker.desktop")), ResultModel::ResourceRole);
        m_defaultModel->appendRow(item);
        item = new QStandardItem();
        item->setData(QUrl(QStringLiteral("kcm:powerdevilprofilesconfig.desktop")), ResultModel::ResourceRole);
        m_defaultModel->appendRow(item);
        item = new QStandardItem();
        item->setData(QUrl(QStringLiteral("kcm:kcm_kscreen.desktop")), ResultModel::ResourceRole);
        m_defaultModel->appendRow(item);
    }

    void setResultModel(ResultModel *model)
    {
        if (m_resultModel == model) {
            return;
        }

        auto updateModel = [this]() {
            if (m_resultModel->rowCount() >= 5) {
                setSourceModel(m_resultModel);
            } else {
                setSourceModel(m_defaultModel);
            }
        };

        m_resultModel = model;

        connect(m_resultModel, &QAbstractItemModel::rowsInserted, this, updateModel);
        connect(m_resultModel, &QAbstractItemModel::rowsRemoved, this, updateModel);

        updateModel();
    }

    QHash<int, QByteArray> roleNames() const
    {
        QHash<int, QByteArray> roleNames;
        roleNames.insert(Qt::DisplayRole, "display");
        roleNames.insert(Qt::DecorationRole, "decoration");
        roleNames.insert(ResultModel::ScoreRole, "score");
        return roleNames;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        MenuItem *mi;
        const QString desktopName = QSortFilterProxyModel::data(index, ResultModel::ResourceRole).toUrl().path();

        if (m_menuItems.contains(desktopName)) {
            mi = m_menuItems.value(desktopName);
        } else {
            mi = new MenuItem(false, nullptr);
            const_cast<MostUsedModel *>(this)->m_menuItems.insert(desktopName, mi);

            KService::Ptr service = KService::serviceByStorageId(desktopName);

            if (!service || !service->isValid()) {
                return QVariant();
            }
            mi->setService(service);
        }

        switch (role) {
            case Qt::UserRole:
                return QVariant::fromValue(mi);
            case Qt::DisplayRole:
                if (mi->service() && mi->service()->isValid()) {
                    return mi->service()->name();
                } else {
                    return QVariant();
                }
            case Qt::DecorationRole:
                if (mi->service() && mi->service()->isValid()) {
                    return mi->service()->icon();
                } else {
                    return QVariant();
                }
            case ResultModel::ScoreRole:
                return QSortFilterProxyModel::data(index, ResultModel::ScoreRole).toInt();
            default:
                return QVariant();
        }
    }

private:
    QHash<QString, MenuItem *> m_menuItems;
    QStandardItemModel *m_defaultModel;
    ResultModel *m_resultModel;
};

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
    SubcategoryModel * subCategoryModel;
    MostUsedModel * mostUsedModel;
    QWidget * mainWidget;
    QQuickWidget * placeHolderWidget;
    QHBoxLayout * mainLayout;
    KDeclarative::KDeclarative kdeclarative;
    MenuProxyModel * categorizedModel;
    MenuProxyModel * searchModel;
    KAboutData * aboutIcon;
    ModuleView * moduleView;
    KActionCollection *collection;
    QPersistentModelIndex activeCategoryIndex;
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
    return d->searchModel;
}

QAbstractItemModel * SidebarMode::subCategoryModel() const
{
    return d->subCategoryModel;
}

QAbstractItemModel * SidebarMode::mostUsedModel() const
{
    return d->mostUsedModel;
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

    d->categorizedModel = new MenuProxyModel( this );
    d->categorizedModel->setCategorizedModel( true );
    d->categorizedModel->setSourceModel( model );
    d->categorizedModel->sort( 0 );
    d->categorizedModel->setFilterHighlightsEntries( false );

    d->searchModel = new MenuProxyModel( this );
    d->searchModel->setFilterHighlightsEntries( false );
    d->searchModel->setSourceModel( d->categorizedModel );
    connect( d->searchModel, &MenuProxyModel::filterRegExpChanged, this, [this] () {
        if (d->activeCategoryIndex.isValid() && d->activeCategoryIndex.row() >= 0) {
            d->subCategoryModel->setParentIndex( d->activeCategoryIndex );
            emit activeCategoryChanged();
        }
    });

    d->mostUsedModel = new MostUsedModel( this );

    d->subCategoryModel = new SubcategoryModel( d->searchModel, this );
    d->mainWidget = new QWidget();
    d->mainWidget->installEventFilter(this);
    d->mainLayout = new QHBoxLayout(d->mainWidget);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->moduleView = new ModuleView( d->mainWidget );
    connect( d->moduleView, &ModuleView::moduleChanged, this, &SidebarMode::moduleLoaded );
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
    if (showToolTips()) {
        d->toolTipManager->requestToolTip(d->searchModel->index(index, 0), rect.toRect());
    }
}

void SidebarMode::hideToolTip()
{
    d->toolTipManager->hideToolTip();
}

Q_INVOKABLE void SidebarMode::loadMostUsed(int index)
{
    const QModelIndex idx = d->mostUsedModel->index(index, 0);
    d->moduleView->closeModules();
    d->moduleView->loadModule( idx );
}

void SidebarMode::changeModule( const QModelIndex& activeModule )
{
    d->moduleView->closeModules();

    const int subRows = d->searchModel->rowCount(activeModule);
    if ( subRows < 2) {
        d->moduleView->loadModule( activeModule );
    } else {
        d->moduleView->loadModule( d->searchModel->index(0, 0, activeModule) );
    }

    d->subCategoryModel->setParentIndex( activeModule );
}

void SidebarMode::moduleLoaded()
{
    d->placeHolderWidget->hide();
    d->moduleView->show();
}

int SidebarMode::activeCategory() const
{
    return d->searchModel->mapFromSource(d->searchModel->sourceModel()->index(d->activeCategory, 0)).row();
}

void SidebarMode::setActiveCategory(int cat)
{
    const QModelIndex idx = d->searchModel->index(cat, 0);
    d->activeCategoryIndex = idx;
    const int newCategoryRow = d->searchModel->mapToSource(idx).row();

    if (d->activeCategory ==newCategoryRow) {
        return;
    }

    d->activeCategoryIndex = idx;
    d->activeCategory = newCategoryRow;

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

    d->quickWidget->setSource(QUrl::fromLocalFile(d->package.filePath("mainscript")));

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

    d->toolTipManager = new ToolTipManager(d->searchModel, d->quickWidget);

    d->placeHolderWidget = new QQuickWidget(d->mainWidget);
    d->placeHolderWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    d->placeHolderWidget->engine()->rootContext()->setContextObject(new KLocalizedContext(d->placeHolderWidget));
    d->placeHolderWidget->engine()->rootContext()->setContextProperty("systemsettings", this);
    d->placeHolderWidget->setSource(QUrl::fromLocalFile(d->package.filePath("ui", "introPage.qml")));

    d->mainLayout->addWidget( d->quickWidget );
    d->moduleView->hide();
    d->mainLayout->addWidget( d->moduleView );
    d->mainLayout->addWidget( d->placeHolderWidget );
    emit changeToolBarItems(BaseMode::NoItems);

    d->mostUsedModel->setResultModel(new ResultModel( AllResources | Agent("org.kde.systemsettings") | HighScoredFirst | Limit(5), this));
}

bool SidebarMode::eventFilter(QObject* watched, QEvent* event)
{
    //TODO: patch Qt
    if (watched == d->quickWidget && event->type() == QEvent::Leave) {
        QCoreApplication::sendEvent(d->quickWidget->quickWindow(), event);
    } else if (watched == d->mainWidget && event->type() == QEvent::Resize) {
        emit widthChanged();
    } else if (watched == d->mainWidget && event->type() == QEvent::Show) {
        emit changeToolBarItems(BaseMode::NoItems);
    }
    return BaseMode::eventFilter(watched, event);
}

void SidebarMode::giveFocus()
{
    d->quickWidget->setFocus();
}

#include "SidebarMode.moc"
