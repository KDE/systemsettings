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

#include "MenuItem.h"
#include "MenuModel.h"
#include "ModuleView.h"
#include "MenuProxyModel.h"
#include "BaseData.h"
#include "ToolTips/tooltipmanager.h"

#include <QGuiApplication>
#include <QHBoxLayout>

#include <QAction>
#include <KAboutData>
#include <KCModuleInfo>
#include <KDescendantsProxyModel>
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
#include <QMenu>
#include <QDebug>

#include <KActivities/Stats/ResultModel>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/Terms>

namespace KAStats = KActivities::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

K_PLUGIN_FACTORY( SidebarModeFactory, registerPlugin<SidebarMode>(); )

FocusHackWidget::FocusHackWidget(QWidget *parent)
    : QWidget(parent)
{}
FocusHackWidget::~FocusHackWidget()
{}

void FocusHackWidget::focusNext()
{
    focusNextChild();
}

void FocusHackWidget::focusPrevious()
{
    focusNextPrevChild(false);
}

SubcategoryModel::SubcategoryModel(QAbstractItemModel *parentModel, QObject *parent)
    : KSelectionProxyModel(nullptr, parent),
        m_parentModel(parentModel)
{
    setSourceModel(parentModel);
    setSelectionModel(new QItemSelectionModel(parentModel, this));
    setFilterBehavior(SubTreesWithoutRoots);
}

QString SubcategoryModel::title() const
{
    return m_title;
}

void SubcategoryModel::setParentIndex(const QModelIndex &activeModule)
{
    selectionModel()->select(activeModule, QItemSelectionModel::ClearAndSelect);
    m_title = activeModule.data(Qt::DisplayRole).toString();
    emit titleChanged();
}


class MostUsedModel : public QSortFilterProxyModel
{
public:
    explicit MostUsedModel(QObject *parent = nullptr)
        : QSortFilterProxyModel (parent)
    {
        sort(0, Qt::DescendingOrder);
        setSortRole(ResultModel::ScoreRole);
        setDynamicSortFilter(true);
        //prepare default items
        m_defaultModel = new QStandardItemModel(this);

        KService::Ptr service = KService::serviceByDesktopName(qGuiApp->desktopFileName());
        if (service) {
            const auto actions = service->actions();
            for (const KServiceAction &action : actions) {
                QStandardItem *item = new QStandardItem();
                item->setData(QUrl(QStringLiteral("kcm:%1.desktop").arg(action.name())), ResultModel::ResourceRole);
                m_defaultModel->appendRow(item);
            }
        } else {
            qCritical() << "Failed to find desktop file for" << qGuiApp->desktopFileName();
        }
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

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roleNames;
        roleNames.insert(Qt::DisplayRole, "display");
        roleNames.insert(Qt::DecorationRole, "decoration");
        roleNames.insert(ResultModel::ScoreRole, "score");
        return roleNames;
    }

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        const QString desktopName = sourceModel()->index(source_row, 0, source_parent).data(ResultModel::ResourceRole).toUrl().path();
        KService::Ptr service = KService::serviceByStorageId(desktopName);
        return service;
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
                qWarning()<<desktopName;
                m_resultModel->forgetResource(QStringLiteral("kcm:") % desktopName);
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
    // Model when there is nothing from kactivities-stat
    QStandardItemModel *m_defaultModel;
    // Model fed by kactivities-stats
    ResultModel *m_resultModel;
};

class SidebarMode::Private {
public:
    Private()
      : quickWidget( nullptr ),
        moduleView( nullptr ),
        collection( nullptr ),
        activeCategoryRow( -1 ),
        activeSubCategoryRow( -1 )
    {}

    virtual ~Private() {
        delete aboutIcon;
    }

    ToolTipManager *toolTipManager = nullptr;
    ToolTipManager *mostUsedToolTipManager = nullptr;
    QQuickWidget * quickWidget = nullptr;
    KPackage::Package package;
    SubcategoryModel * subCategoryModel = nullptr;
    MostUsedModel * mostUsedModel = nullptr;
    FocusHackWidget * mainWidget = nullptr;
    QQuickWidget * placeHolderWidget = nullptr;
    QHBoxLayout * mainLayout = nullptr;
    KDeclarative::KDeclarative kdeclarative;
    MenuModel * model = nullptr;
    MenuProxyModel * categorizedModel = nullptr;
    MenuProxyModel * searchModel = nullptr;
    KDescendantsProxyModel * flatModel = nullptr;
    KAboutData * aboutIcon = nullptr;
    ModuleView * moduleView = nullptr;
    KActionCollection *collection = nullptr;
    QPersistentModelIndex activeCategoryIndex;
    int activeCategoryRow = -1;
    int activeSubCategoryRow = -1;
    int activeSearchRow = -1;
    qreal headerHeight = 0;
    bool m_actionMenuVisible = false;
    void setActionMenuVisible(SidebarMode* sidebarMode, const bool &actionMenuVisible)
    {
        if (m_actionMenuVisible == actionMenuVisible) {
            return;
        }
        m_actionMenuVisible = actionMenuVisible;
        emit sidebarMode->actionMenuVisibleChanged();
    }
    bool m_introPageVisible = true;
    bool m_defaultsIndicatorsVisible = false;
};

SidebarMode::SidebarMode( QObject *parent, const QVariantList &args )
    : BaseMode( parent, args )
    , d( new Private() )
{
    qApp->setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    d->aboutIcon = new KAboutData( QStringLiteral("SidebarView"), i18n( "Sidebar View" ),
                                 QStringLiteral("1.0"), i18n( "Provides a categorized sidebar for control modules." ),
                                 KAboutLicense::GPL, i18n( "(c) 2017, Marco Martin" ) );
    d->aboutIcon->addAuthor( i18n( "Marco Martin" ), i18n( "Author" ), QStringLiteral("mart@kde.org") );
    d->aboutIcon->addAuthor( i18n( "Ben Cooksley" ), i18n( "Author" ), QStringLiteral("bcooksley@kde.org") );
    d->aboutIcon->addAuthor( i18n( "Mathias Soeken" ), i18n( "Developer" ), QStringLiteral("msoeken@informatik.uni-bremen.de") );

    qmlRegisterType<QAction>();
    qmlRegisterType<QAbstractItemModel>();
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
    return d->categorizedModel;
}

QAbstractItemModel * SidebarMode::searchModel() const
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
    d->model = new MenuModel( rootItem(), this );
    foreach( MenuItem * child, rootItem()->children() ) {
        d->model->addException( child );
    }

    d->categorizedModel = new MenuProxyModel( this );
    d->categorizedModel->setCategorizedModel( true );
    d->categorizedModel->setSourceModel( d->model );
    d->categorizedModel->sort( 0 );
    d->categorizedModel->setFilterHighlightsEntries( false );

    d->flatModel = new KDescendantsProxyModel( this );
    d->flatModel->setSourceModel( d->model );

    d->searchModel = new MenuProxyModel( this );
    d->searchModel->setCategorizedModel( true );
    d->searchModel->setFilterHighlightsEntries( false );
    d->searchModel->setSourceModel( d->flatModel );

    d->mostUsedModel = new MostUsedModel( this );

    d->subCategoryModel = new SubcategoryModel( d->categorizedModel, this );
    d->mainWidget = new FocusHackWidget();
    d->mainWidget->installEventFilter(this);
    d->mainLayout = new QHBoxLayout(d->mainWidget);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->mainLayout->setSpacing(0);
    d->moduleView = new ModuleView( d->mainWidget );
    connect( d->moduleView, &ModuleView::moduleChanged, this, &SidebarMode::moduleLoaded );
    connect(d->moduleView, &ModuleView::moduleSaved, this, &SidebarMode::updateDefaults);
    d->quickWidget = nullptr;
    moduleView()->setFaceType(KPageView::Plain);
    if (applicationMode() == BaseMode::InfoCenter) {
        d->moduleView->setSaveStatistics(false);
        d->moduleView->setApplyVisible(false);
        d->moduleView->setDefaultsVisible(false);
        d->moduleView->setResetVisible(false);
    }
}

QAction *SidebarMode::action(const QString &name) const
{
    if (!d->collection) {
        return nullptr;
    }

    return d->collection->action(name);
}

QString SidebarMode::actionIconName(const QString &name) const
{
    if (QAction *a = action(name)) {
        return a->icon().name();
    }

    return QString();
}

void SidebarMode::requestToolTip(const QModelIndex &index, const QRectF &rect)
{
    if (showToolTips() && index.model()) {
        d->toolTipManager->setModel(index.model());
        d->toolTipManager->requestToolTip(index, rect.toRect());
    }
}

void SidebarMode::requestMostUsedToolTip(int index, const QRectF &rect)
{
    if (showToolTips()) {
        d->mostUsedToolTipManager->requestToolTip(d->mostUsedModel->index(index, 0), rect.toRect());
    }
}

void SidebarMode::hideToolTip()
{
    d->toolTipManager->hideToolTip();
}

void SidebarMode::hideMostUsedToolTip()
{
    d->mostUsedToolTipManager->hideToolTip();
}

void SidebarMode::showActionMenu(const QPoint &position)
{
    QMenu *menu = new QMenu();
    connect(menu, &QMenu::aboutToHide, this, [this] () { d->setActionMenuVisible(this, false); } );
    menu->setAttribute(Qt::WA_DeleteOnClose);

    const QStringList actionList { QStringLiteral("configure"), QStringLiteral("help_contents"), QStringLiteral("help_report_bug"), QStringLiteral("help_about_app"), QStringLiteral("help_about_kde") };
    for (const QString &actionName : actionList) {
        menu->addAction(d->collection->action(actionName));
    }

    menu->popup(position);
    d->setActionMenuVisible(this, true);
}

void SidebarMode::loadModule( const QModelIndex& activeModule, const QStringList &args )
{
    if (!activeModule.isValid()) {
        return;
    }

    MenuItem *mi = activeModule.data(MenuModel::MenuItemRole).value<MenuItem *>();

    if (!mi) {
        return;
    }

    // If we are trying to load a module already open
    if (d->moduleView->activeModule() && mi->item() == *d->moduleView->activeModule()) {
        return;
    }

    if( !d->moduleView->resolveChanges() ) {
        return;
    }

    d->moduleView->closeModules();

    if (homeItem()) {
        d->m_introPageVisible = activeModule == d->categorizedModel->mapFromSource(d->model->indexForItem(homeItem()));
        emit introPageVisibleChanged();
    } else {
        setIntroPageVisible(false);
    }

    if ( mi->children().length() < 1) {
        d->moduleView->loadModule( activeModule, args );
    } else {
        d->moduleView->loadModule( activeModule.model()->index(0, 0, activeModule), args );
    }

    if (activeModule.model() == d->categorizedModel) {
        const int newCategoryRow = activeModule.row();

        if (d->activeCategoryRow == newCategoryRow) {
            return;
        }

        d->activeCategoryIndex = activeModule;
        d->activeCategoryRow = newCategoryRow;

        d->activeSubCategoryRow = 0;

        d->subCategoryModel->setParentIndex( activeModule );

        if (d->activeSearchRow > -1) {
            d->activeSearchRow = -1;
            emit activeSearchRowChanged();
        }

        emit activeCategoryRowChanged();
        emit activeSubCategoryRowChanged();

    } else if (activeModule.model() == d->subCategoryModel) {
        if (d->activeSearchRow > -1) {
            d->activeSearchRow = -1;
            emit activeSearchRowChanged();
        }
        d->activeSubCategoryRow = activeModule.row();
        emit activeSubCategoryRowChanged();

    } else if (activeModule.model() == d->searchModel) {
        QModelIndex originalIndex = d->categorizedModel->mapFromSource(
            d->flatModel->mapToSource(
                d->searchModel->mapToSource(activeModule)));

        if (originalIndex.isValid()) {
            //are we in a  subcategory of the top categories?
            if (originalIndex.parent().isValid() && mi->parent()->menu()) {
                d->activeCategoryRow = originalIndex.parent().row();
                d->activeSubCategoryRow = originalIndex.row();

            // Is this kcm directly at the top level without a top category?
            } else {
                d->activeCategoryRow = originalIndex.row();
                d->activeSubCategoryRow = -1;
            }

            d->subCategoryModel->setParentIndex( originalIndex.parent() );
            emit activeCategoryRowChanged();
            emit activeSubCategoryRowChanged();
        }

        d->activeSearchRow = activeModule.row();
        emit activeSearchRowChanged();

    } else {
        if (d->activeSearchRow > -1) {
            d->activeSearchRow = -1;
            emit activeSearchRowChanged();
        }

        QModelIndex flatIndex;

        // search the corresponding item on the main model
        for (int i = 0; i < d->flatModel->rowCount(); ++i) {
            QModelIndex idx = d->flatModel->index(i, 0);
            MenuItem *otherMi = idx.data(MenuModel::MenuItemRole).value<MenuItem *>();

            if (otherMi->item() == mi->item()) {
                flatIndex = idx;
                break;
            }
        }

        if (flatIndex.isValid()) {
            QModelIndex idx = d->categorizedModel->mapFromSource(d->flatModel->mapToSource(flatIndex));

            MenuItem *parentMi = idx.parent().data(MenuModel::MenuItemRole).value<MenuItem *>();
            if (idx.isValid()) {
                if (parentMi && parentMi->menu()) {
                    d->subCategoryModel->setParentIndex( idx.parent() );
                    d->activeCategoryRow = idx.parent().row();
                    d->activeSubCategoryRow = idx.row();
                } else {
                    d->activeCategoryRow = idx.row();
                    d->activeSubCategoryRow = -1;
                }
                emit activeCategoryRowChanged();
                emit activeSubCategoryRowChanged();
            }
        }
    }
}

void SidebarMode::moduleLoaded()
{
    d->placeHolderWidget->hide();
    d->moduleView->show();
    if (applicationMode() == BaseMode::InfoCenter) {
        d->moduleView->setSaveStatistics(false);
        d->moduleView->setApplyVisible(false);
        d->moduleView->setDefaultsVisible(false);
        d->moduleView->setResetVisible(false);
    }
}

void SidebarMode::updateDefaults()
{
    QModelIndex categoryIdx = d->categorizedModel->index(d->activeCategoryRow, 0);
    auto item = categoryIdx.data(Qt::UserRole).value<MenuItem*>();
    Q_ASSERT(item);
    // If subcategory exist update from subcategory
    if (!item->children().isEmpty()) {
        auto subCateogryIdx = d->subCategoryModel->index(d->activeSubCategoryRow, 0);
        item = subCateogryIdx.data(Qt::UserRole).value<MenuItem*>();
    }
    item->updateDefaultIndicator();

    auto sourceIdx = d->categorizedModel->mapToSource(categoryIdx);
    emit d->model->dataChanged(sourceIdx, sourceIdx);

    auto subCateogryIdx = d->subCategoryModel->index(d->activeSubCategoryRow, 0);
    auto subCategorySourceIdx = d->categorizedModel->mapToSource(d->subCategoryModel->mapToSource(subCateogryIdx));
    emit d->model->dataChanged(subCategorySourceIdx, subCategorySourceIdx);
}

int SidebarMode::activeSearchRow() const
{
    return d->activeSearchRow;
}

int SidebarMode::activeCategoryRow() const
{
    return d->activeCategoryRow;
}

void SidebarMode::setIntroPageVisible(const bool &introPageVisible)
{
    if (d->m_introPageVisible == introPageVisible) {
        return;
    }

    // TODO: Make the intro page of SystemSettings a KCM as well
    if (homeItem()) {
        d->placeHolderWidget->hide();
        d->moduleView->show();
        if (introPageVisible) {
            loadModule(d->categorizedModel->mapFromSource(d->model->indexForItem(homeItem())));
        }
    } else {
        if (introPageVisible) {
            d->subCategoryModel->setParentIndex(QModelIndex());
            d->activeCategoryRow = -1;
            emit activeCategoryRowChanged();
            d->activeSubCategoryRow = -1;
            emit activeSubCategoryRowChanged();
            d->placeHolderWidget->show();
            d->moduleView->hide();
        } else {
            d->placeHolderWidget->hide();
            d->moduleView->show();
        }
    }

    d->m_introPageVisible = introPageVisible;
    emit introPageVisibleChanged();
}

void SidebarMode::setHeaderHeight(qreal height)
{
    if (height == d->moduleView->headerHeight()) {
        return;
    }

    d->moduleView->setHeaderHeight(height);
    emit headerHeightChanged();
}

qreal SidebarMode::headerHeight() const
{
    return d->moduleView->headerHeight();
}

void SidebarMode::toggleDefaultsIndicatorsVisibility()
{
    d->m_defaultsIndicatorsVisible = !d->m_defaultsIndicatorsVisible;
    d->moduleView->moduleShowDefaultsIndicators(d->m_defaultsIndicatorsVisible);

    if (d->m_defaultsIndicatorsVisible) {
        for (int i = 0; i < d->flatModel->rowCount(); ++i) {
            QModelIndex idx = d->flatModel->index(i, 0);
            auto item = idx.data(MenuModel::MenuItemRole).value<MenuItem *>();
            if (item->menu()) {
                continue;
            }
            const bool showIndicator = item->showDefaultIndicator();
            item->updateDefaultIndicator();
            if (showIndicator == item->showDefaultIndicator()) {
                continue;
            }
            auto itemIdx = d->model->indexForItem(item);
            emit d->model->dataChanged(itemIdx, itemIdx);
            MenuItem *parent = item->parent();
            while (parent) {
                auto parentIdx = d->model->indexForItem(parent);
                if (parentIdx.isValid()) {
                    emit d->model->dataChanged(parentIdx, parentIdx);
                    parent = parent->parent();
                } else {
                    parent = nullptr;
                }
            }
        }
    }

    emit defaultsIndicatorsVisibleChanged();
}

int SidebarMode::width() const
{
    return d->mainWidget->width();
}

bool SidebarMode::actionMenuVisible() const
{
    return d->m_actionMenuVisible;
}

int SidebarMode::activeSubCategoryRow() const
{
    return d->activeSubCategoryRow;
}

bool SidebarMode::introPageVisible() const
{
    return (d->m_introPageVisible);
}

bool SidebarMode::defaultsIndicatorsVisible() const
{
    return d->m_defaultsIndicatorsVisible;
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
    d->quickWidget->quickWindow()->setTitle(i18n("Sidebar"));
    d->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlRegisterUncreatableType<SidebarMode>("org.kde.systemsettings", 1, 0, "SystemSettings", QStringLiteral("Not creatable, use the systemsettings attached property"));

    d->quickWidget->engine()->rootContext()->setContextProperty(QStringLiteral("systemsettings"), this);
    d->package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("KPackage/GenericQML"));
    d->package.setPath(QStringLiteral("org.kde.systemsettings.sidebar"));

    d->kdeclarative.setDeclarativeEngine(d->quickWidget->engine());
    d->kdeclarative.setupEngine(d->quickWidget->engine());
    d->kdeclarative.setupContext();

    d->quickWidget->setSource(QUrl::fromLocalFile(d->package.filePath("mainscript")));

    if (!d->quickWidget->rootObject()) {
        for (const auto &err : d->quickWidget->errors()) {
            qWarning() << err.toString();
        }
        qFatal("Fatal error while loading the sidebar view qml component");
    }
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
    connect(d->quickWidget->rootObject(), SIGNAL(focusNextRequest()), d->mainWidget, SLOT(focusNext()));
    connect(d->quickWidget->rootObject(), SIGNAL(focusPreviousRequest()), d->mainWidget, SLOT(focusPrevious()));

    d->quickWidget->installEventFilter(this);

    d->placeHolderWidget = new QQuickWidget(d->mainWidget);
    d->placeHolderWidget->quickWindow()->setTitle(i18n("Most Used"));
    d->placeHolderWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    d->placeHolderWidget->engine()->rootContext()->setContextObject(new KLocalizedContext(d->placeHolderWidget));
    d->placeHolderWidget->engine()->rootContext()->setContextProperty(QStringLiteral("systemsettings"), this);
    d->placeHolderWidget->setSource(QUrl::fromLocalFile(d->package.filePath("ui", QStringLiteral("introPage.qml"))));
    connect(d->placeHolderWidget->rootObject(), SIGNAL(focusNextRequest()), d->mainWidget, SLOT(focusNext()));
    connect(d->placeHolderWidget->rootObject(), SIGNAL(focusPreviousRequest()), d->mainWidget, SLOT(focusPrevious()));
    d->placeHolderWidget->installEventFilter(this);

    d->mainLayout->addWidget( d->quickWidget );
    d->moduleView->hide();
    d->mainLayout->addWidget( d->moduleView );
    d->mainLayout->addWidget( d->placeHolderWidget );
    emit changeToolBarItems(BaseMode::NoItems);

    d->toolTipManager = new ToolTipManager(d->categorizedModel, d->quickWidget, ToolTipManager::ToolTipPosition::Right);
    d->mostUsedToolTipManager = new ToolTipManager(d->mostUsedModel, d->placeHolderWidget, ToolTipManager::ToolTipPosition::BottomCenter);

    d->mostUsedModel->setResultModel(new ResultModel( AllResources | Agent(QStringLiteral("org.kde.systemsettings")) | HighScoredFirst | Limit(5), this));

    if (homeItem()) {
        d->placeHolderWidget->hide();
        d->moduleView->show();
        loadModule(d->categorizedModel->mapFromSource(d->model->indexForItem(homeItem())));
    }

    if (!startupModule().isEmpty()) {
        MenuItem *item = rootItem()->descendantForModule(startupModule());
        if (item) {
            loadModule(d->model->indexForItem(item), startupModuleArgs());
        }
    }
}

void SidebarMode::reloadStartupModule()
{
    if (!startupModule().isEmpty()) {
        MenuItem *item = rootItem()->descendantForModule(startupModule());
        if (item) {
            loadModule(d->model->indexForItem(item), startupModuleArgs());
        }
    }
}

bool SidebarMode::eventFilter(QObject* watched, QEvent* event)
{
    //FIXME: those are all workarounds around the QQuickWidget brokeness
    if ((watched == d->quickWidget || watched == d->placeHolderWidget)
         && event->type() == QEvent::KeyPress) {
        //allow tab navigation inside the qquickwidget
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        QQuickWidget *qqw = static_cast<QQuickWidget *>(watched);
        if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
            QCoreApplication::sendEvent(qqw->quickWindow(), event);
            return true;
        }
    } else if ((watched == d->quickWidget || watched == d->placeHolderWidget)
                && event->type() == QEvent::FocusIn) {
        QFocusEvent *fe = static_cast<QFocusEvent *>(event);
        QQuickWidget *qqw = static_cast<QQuickWidget *>(watched);
        if (qqw && qqw->rootObject()) {
            if (fe->reason() == Qt::TabFocusReason) {
                QMetaObject::invokeMethod(qqw->rootObject(), "focusFirstChild");
            } else if (fe->reason() == Qt::BacktabFocusReason) {
                QMetaObject::invokeMethod(qqw->rootObject(), "focusLastChild");
            }
        }
    } else if (watched == d->quickWidget && event->type() == QEvent::Leave) {
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
