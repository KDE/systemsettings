/*
 * SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "IconsMode.h"

#include "BaseData.h"
#include "MenuItem.h"
#include "MenuModel.h"
#include "MenuProxyModel.h"
#include "ModuleView.h"
#include "ToolTips/tooltipmanager.h"

#include <QGuiApplication>
#include <QHBoxLayout>

#include <KAboutData>
#include <KActionCollection>
#include <KCModuleInfo>
#include <KCMUtils/KCModuleLoader>
#include <KConfigGroup>
#include <KDescendantsProxyModel>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KXmlGuiWindow>
#include <QAction>
#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QMenu>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QStandardItemModel>
#include <QStackedWidget>

#include <KActivities/Stats/ResultModel>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/Terms>

namespace KAStats = KActivities::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

K_PLUGIN_CLASS_WITH_JSON(IconsMode, "settings-iconsquick-view.json")

FocusHackWidget::FocusHackWidget(QWidget *parent)
    : QWidget(parent)
{
}

FocusHackWidget::~FocusHackWidget()
{
}

void FocusHackWidget::focusNext()
{
    focusNextChild();
}

void FocusHackWidget::focusPrevious()
{
    focusNextPrevChild(false);
}

SubcategoryModel::SubcategoryModel(QAbstractItemModel *parentModel, IconsMode *parent)
    : KSelectionProxyModel(nullptr, parent)
    , m_parentModel(parentModel)
    , m_iconsMode(parent)
{
    setSourceModel(parentModel);
    setSelectionModel(new QItemSelectionModel(parentModel, this));
    setFilterBehavior(SubTreesWithoutRoots);
}

QString SubcategoryModel::title() const
{
    MenuItem *mi = m_activeModuleIndex.data(MenuModel::MenuItemRole).value<MenuItem *>();

    if (!mi) {
        return QString();
    }

    return mi->name();
}

QIcon SubcategoryModel::icon() const
{
    return m_activeModuleIndex.data(Qt::DecorationRole).value<QIcon>();
}

bool SubcategoryModel::categoryOwnedByKCM() const
{
    return m_activeModuleIndex.data(MenuModel::IsKCMRole).toBool();
}

void SubcategoryModel::setParentIndex(const QModelIndex &activeModule)
{
    selectionModel()->select(activeModule, QItemSelectionModel::ClearAndSelect);
    m_activeModuleIndex = QPersistentModelIndex(activeModule);
    Q_EMIT titleChanged();
    Q_EMIT iconChanged();
    Q_EMIT categoryOwnedByKCMChanged();
}

void SubcategoryModel::loadParentCategoryModule()
{
    MenuItem *menuItem = m_activeModuleIndex.data(MenuModel::MenuItemRole).value<MenuItem *>();
    if (!menuItem->item().library().isEmpty()) {
        m_iconsMode->loadModule(m_activeModuleIndex);
    }
}

class MostUsedModel : public QSortFilterProxyModel
{
public:
    explicit MostUsedModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        sort(0, Qt::DescendingOrder);
        setSortRole(ResultModel::ScoreRole);
        setDynamicSortFilter(true);
        // prepare default items
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
                qWarning() << desktopName;
                m_resultModel->forgetResource(QStringLiteral("kcm:") % desktopName);
                return QVariant();
            }
            mi->setService(service);
        }

        switch (role) {
        case Qt::UserRole:
            return QVariant::fromValue(mi);
        case Qt::DisplayRole:
            return mi->name();
        case Qt::DecorationRole:
            return mi->iconName();
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

class IconsMode::Private
{
public:
    Private()
        : quickWidget(nullptr)
        , moduleView(nullptr)
        , collection(nullptr)
        , activeCategoryRow(-1)
        , activeSubCategoryRow(-1)
    {
    }

    virtual ~Private()
    {
        delete aboutIcon;
    }

    ToolTipManager *toolTipManager = nullptr;
    QQuickWidget *quickWidget = nullptr;

    QQuickWidget *toolbarWidget = nullptr;

    QWidget *bottomWidget = nullptr;
    QHBoxLayout *bottomLayout = nullptr;

    KPackage::Package package;
    SubcategoryModel *subCategoryModel = nullptr;
    MostUsedModel *mostUsedModel = nullptr;
    FocusHackWidget *mainWidget = nullptr;
    QVBoxLayout *mainLayout = nullptr;
    MenuModel *model = nullptr;
    MenuProxyModel *categorizedModel = nullptr;
    MenuProxyModel *searchModel = nullptr;
    KDescendantsProxyModel *flatModel = nullptr;
    KAboutData *aboutIcon = nullptr;
    ModuleView *moduleView = nullptr;
    KActionCollection *collection = nullptr;
    QPersistentModelIndex activeCategoryIndex;
    int activeCategoryRow = -1;
    int activeSubCategoryRow = -1;
    int activeSearchRow = -1;
    qreal headerHeight = 0;
    bool m_actionMenuVisible = false;
    void setActionMenuVisible(IconsMode *iconsView, const bool &actionMenuVisible)
    {
        if (m_actionMenuVisible == actionMenuVisible) {
            return;
        }
        m_actionMenuVisible = actionMenuVisible;
        Q_EMIT iconsView->actionMenuVisibleChanged();
    }
    bool m_defaultsIndicatorsVisible = false;
};

IconsMode::IconsMode(QObject *parent, const QVariantList &args)
    : BaseMode(parent, args)
    , d(new Private())
{
    qApp->setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    d->aboutIcon = new KAboutData(QStringLiteral("IconsViewQuick"),
                                  i18n("Icon View"),
                                  QStringLiteral("1.0"),
                                  i18n("Provides a categorized icon for control modules."),
                                  KAboutLicense::GPL,
                                  i18n("(c) 2012, Janet Blackquill"));
    d->aboutIcon->addAuthor(i18n("Janet Blackquill"), i18n("Author"), QStringLiteral("uhhadd@gmail.com"));

    qmlRegisterType<QAction>();
    qmlRegisterType<QAbstractItemModel>();
}

IconsMode::~IconsMode()
{
    config().sync();
    delete d;
}

KAboutData *IconsMode::aboutData()
{
    return d->aboutIcon;
}

ModuleView *IconsMode::moduleView() const
{
    return d->moduleView;
}

QWidget *IconsMode::mainWidget()
{
    if (!d->quickWidget) {
        initWidget();
    }
    return d->mainWidget;
}

QAbstractItemModel *IconsMode::categoryModel() const
{
    return d->categorizedModel;
}

QAbstractItemModel *IconsMode::searchModel() const
{
    return d->searchModel;
}

QAbstractItemModel *IconsMode::subCategoryModel() const
{
    return d->subCategoryModel;
}

QAbstractItemModel *IconsMode::mostUsedModel() const
{
    return d->mostUsedModel;
}

QList<QAbstractItemView *> IconsMode::views() const
{
    QList<QAbstractItemView *> list;
    // list.append( d->categoryView );
    return list;
}

void IconsMode::initEvent()
{
    d->model = new MenuModel(rootItem(), this);
    foreach (MenuItem *child, rootItem()->children()) {
        d->model->addException(child);
    }

    d->categorizedModel = new MenuProxyModel(this);
    d->categorizedModel->setCategorizedModel(true);
    d->categorizedModel->setSourceModel(d->model);
    d->categorizedModel->sort(0);
    d->categorizedModel->setFilterHighlightsEntries(false);

    d->flatModel = new KDescendantsProxyModel(this);
    d->flatModel->setSourceModel(d->model);

    d->searchModel = new MenuProxyModel(this);
    d->searchModel->setCategorizedModel(true);
    d->searchModel->setFilterHighlightsEntries(false);
    d->searchModel->setSourceModel(d->flatModel);

    d->mostUsedModel = new MostUsedModel(this);

    d->subCategoryModel = new SubcategoryModel(d->categorizedModel, this);

    d->mainWidget = new FocusHackWidget();
    d->mainWidget->installEventFilter(this);

    d->mainLayout = new QVBoxLayout(d->mainWidget);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->mainLayout->setSpacing(0);

    d->bottomWidget = new QWidget();

    d->bottomLayout = new QHBoxLayout(d->bottomWidget);
    d->bottomLayout->setContentsMargins(0, 0, 0, 0);
    d->bottomLayout->setSpacing(0);

    d->moduleView = new ModuleView(d->mainWidget);
    connect(d->moduleView, &ModuleView::moduleChanged, this, &IconsMode::moduleLoaded);
    connect(d->moduleView, &ModuleView::moduleSaved, this, &IconsMode::updateDefaults);
    d->quickWidget = nullptr;
    moduleView()->setFaceType(KPageView::Plain);
    if (applicationMode() == BaseMode::InfoCenter) {
        d->moduleView->setSaveStatistics(false);
        d->moduleView->setApplyVisible(false);
        d->moduleView->setDefaultsVisible(false);
        d->moduleView->setResetVisible(false);
    }

    if (config().readEntry("HighlightNonDefaultSettings", false)) {
        toggleDefaultsIndicatorsVisibility();
    }
}

QAction *IconsMode::action(const QString &name) const
{
    if (!d->collection) {
        return nullptr;
    }

    return d->collection->action(name);
}

QString IconsMode::actionIconName(const QString &name) const
{
    if (QAction *a = action(name)) {
        return a->icon().name();
    }

    return QString();
}

void IconsMode::requestToolTip(const QModelIndex &index, const QRectF &rect)
{
    if (showToolTips() && index.model()) {
        d->toolTipManager->setModel(index.model());
        d->toolTipManager->requestToolTip(index, rect.toRect());
    }
}

void IconsMode::hideToolTip()
{
    if (d->toolTipManager) {
        d->toolTipManager->hideToolTip();
    }
}

void IconsMode::showActionMenu(const QPoint &position)
{
    QMenu *menu = new QMenu();
    connect(menu, &QMenu::aboutToHide, this, [this]() {
        d->setActionMenuVisible(this, false);
    });
    menu->setAttribute(Qt::WA_DeleteOnClose);

    const QStringList actionList{QStringLiteral("configure"),
                                 QStringLiteral("help_contents"),
                                 QStringLiteral("help_report_bug"),
                                 QStringLiteral("help_about_app"),
                                 QStringLiteral("help_about_kde")};
    for (const QString &actionName : actionList) {
        menu->addAction(d->collection->action(actionName));
    }

    menu->popup(position);
    d->setActionMenuVisible(this, true);
}

void IconsMode::loadModule(const QModelIndex &activeModule, const QStringList &args)
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

    if (!d->moduleView->resolveChanges()) {
        return;
    }

    d->moduleView->closeModules();

    d->moduleView->loadModule(activeModule, args);

    if (activeModule.model() == d->categorizedModel) {
        const int newCategoryRow = activeModule.row();

        d->activeCategoryIndex = activeModule;
        d->activeCategoryRow = newCategoryRow;

        d->activeSubCategoryRow = 0;

        d->subCategoryModel->setParentIndex(activeModule);

        if (d->activeSearchRow > -1) {
            d->activeSearchRow = -1;
            Q_EMIT activeSearchRowChanged();
        }
        Q_EMIT activeCategoryRowChanged();
        Q_EMIT activeSubCategoryRowChanged();

    } else if (activeModule.model() == d->subCategoryModel) {
        if (d->activeSearchRow > -1) {
            d->activeSearchRow = -1;
            Q_EMIT activeSearchRowChanged();
        }
        d->activeSubCategoryRow = activeModule.row();
        Q_EMIT activeSubCategoryRowChanged();

    } else if (activeModule.model() == d->searchModel) {
        QModelIndex originalIndex = d->categorizedModel->mapFromSource(d->flatModel->mapToSource(d->searchModel->mapToSource(activeModule)));

        if (originalIndex.isValid()) {
            // are we in a  subcategory of the top categories?
            if (originalIndex.parent().isValid() && mi->parent()->menu()) {
                d->activeCategoryRow = originalIndex.parent().row();
                d->activeSubCategoryRow = originalIndex.row();

                // Is this kcm directly at the top level without a top category?
            } else {
                d->activeCategoryRow = originalIndex.row();
                d->activeSubCategoryRow = -1;
            }

            d->subCategoryModel->setParentIndex(originalIndex.parent().isValid() ? originalIndex.parent() : originalIndex);
            Q_EMIT activeCategoryRowChanged();
            Q_EMIT activeSubCategoryRowChanged();
        }

        d->activeSearchRow = activeModule.row();
        Q_EMIT activeSearchRowChanged();

    } else {
        if (d->activeSearchRow > -1) {
            d->activeSearchRow = -1;
            Q_EMIT activeSearchRowChanged();
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
                    d->subCategoryModel->setParentIndex(idx.parent());
                    d->activeCategoryRow = idx.parent().row();
                    d->activeSubCategoryRow = idx.row();
                } else {
                    if (d->categorizedModel->rowCount(idx) > 0) {
                        d->subCategoryModel->setParentIndex(idx);
                    }
                    d->activeCategoryRow = idx.row();
                    d->activeSubCategoryRow = -1;
                }
                Q_EMIT activeCategoryRowChanged();
                Q_EMIT activeSubCategoryRowChanged();
            }
        }
    }
}

void IconsMode::moduleLoaded()
{
    d->quickWidget->setMaximumWidth(240);
    d->moduleView->show();
    if (applicationMode() == BaseMode::InfoCenter) {
        d->moduleView->setSaveStatistics(false);
        d->moduleView->setApplyVisible(false);
        d->moduleView->setDefaultsVisible(false);
        d->moduleView->setResetVisible(false);
    }
}

void IconsMode::updateDefaults()
{
    // When the landing page is loaded, we don't have activeCategoryRow and need to reload everything
    if (d->activeCategoryRow < 0) {
        refreshDefaults();
        return;
    }
    QModelIndex categoryIdx = d->categorizedModel->index(d->activeCategoryRow, 0);
    auto item = categoryIdx.data(Qt::UserRole).value<MenuItem *>();
    Q_ASSERT(item);

    // If subcategory exist update from subcategory, unless this category is owned by a kcm
    if (!item->children().isEmpty() && d->activeSubCategoryRow > -1) {
        auto subCateogryIdx = d->subCategoryModel->index(d->activeSubCategoryRow, 0);
        item = subCateogryIdx.data(Qt::UserRole).value<MenuItem *>();
    }
    // In case we are updating a parent like LnF we refresh all defaults
    if (item->isCategoryOwner() && item->parent() != rootItem()) {
        refreshDefaults();
        return;
    }

    auto *moduleData = KCModuleLoader::loadModuleData(item->item());
    if (moduleData) {
        connect(moduleData, &KCModuleData::loaded, this, [this, item, moduleData, categoryIdx]() {
            item->setDefaultIndicator(!moduleData->isDefaults());
            updateCategoryModel(categoryIdx);
            moduleData->deleteLater();
        });
    }
}

void IconsMode::updateCategoryModel(const QModelIndex &categoryIdx)
{
    auto sourceIdx = d->categorizedModel->mapToSource(categoryIdx);
    Q_EMIT d->model->dataChanged(sourceIdx, sourceIdx);

    auto subCateogryIdx = d->subCategoryModel->index(d->activeSubCategoryRow, 0);
    auto subCategorySourceIdx = d->categorizedModel->mapToSource(d->subCategoryModel->mapToSource(subCateogryIdx));
    Q_EMIT d->model->dataChanged(subCategorySourceIdx, subCategorySourceIdx);
}

int IconsMode::activeSearchRow() const
{
    return d->activeSearchRow;
}

int IconsMode::activeCategoryRow() const
{
    return d->activeCategoryRow;
}

void IconsMode::setHeaderHeight(qreal height)
{
    if (height == d->moduleView->headerHeight()) {
        return;
    }

    d->moduleView->setHeaderHeight(height);
    Q_EMIT headerHeightChanged();
}

qreal IconsMode::headerHeight() const
{
    return d->moduleView->headerHeight();
}

void IconsMode::refreshDefaults()
{
    if (d->m_defaultsIndicatorsVisible) {
        for (int i = 0; i < d->flatModel->rowCount(); ++i) {
            QModelIndex idx = d->flatModel->index(i, 0);
            auto item = idx.data(MenuModel::MenuItemRole).value<MenuItem *>();
            if (item->menu()) {
                continue;
            }

            auto *moduleData = KCModuleLoader::loadModuleData(item->item());
            if (moduleData) {
                connect(moduleData, &KCModuleData::loaded, this, [this, item, moduleData]() {
                    item->setDefaultIndicator(!moduleData->isDefaults());
                    updateModelMenuItem(item);
                    moduleData->deleteLater();
                });
            }
        }
    }
}
void IconsMode::toggleDefaultsIndicatorsVisibility()
{
    d->m_defaultsIndicatorsVisible = !d->m_defaultsIndicatorsVisible;
    d->moduleView->moduleShowDefaultsIndicators(d->m_defaultsIndicatorsVisible);
    refreshDefaults();
    config().writeEntry("HighlightNonDefaultSettings", d->m_defaultsIndicatorsVisible);
    Q_EMIT defaultsIndicatorsVisibleChanged();
}

void IconsMode::updateModelMenuItem(MenuItem *item)
{
    auto itemIdx = d->model->indexForItem(item);
    Q_EMIT d->model->dataChanged(itemIdx, itemIdx);
    MenuItem *parent = item->parent();
    while (parent) {
        auto parentIdx = d->model->indexForItem(parent);
        if (parentIdx.isValid()) {
            Q_EMIT d->model->dataChanged(parentIdx, parentIdx);
            parent = parent->parent();
        } else {
            parent = nullptr;
        }
    }
}

int IconsMode::width() const
{
    return d->mainWidget->width();
}

bool IconsMode::actionMenuVisible() const
{
    return d->m_actionMenuVisible;
}

int IconsMode::activeSubCategoryRow() const
{
    return d->activeSubCategoryRow;
}

bool IconsMode::defaultsIndicatorsVisible() const
{
    return d->m_defaultsIndicatorsVisible;
}

void IconsMode::initWidget()
{
    // Create the widgets

    if (!KMainWindow::memberList().isEmpty()) {
        KXmlGuiWindow *mainWindow = qobject_cast<KXmlGuiWindow *>(KMainWindow::memberList().first());
        if (mainWindow) {
            d->collection = mainWindow->actionCollection();
        }
    }

    d->quickWidget = new QQuickWidget(d->mainWidget);
    d->quickWidget->quickWindow()->setTitle(i18n("Icons"));
    d->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlRegisterUncreatableType<IconsMode>(
        "org.kde.systemsettings", 1, 0, "SystemSettings", QStringLiteral("Not creatable, use the systemsettings attached property"));

    d->quickWidget->engine()->rootContext()->setContextProperty(QStringLiteral("systemsettings"), this);
    d->package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("KPackage/GenericQML"));
    d->package.setPath(QStringLiteral("org.kde.systemsettings.iconsquick"));

    d->quickWidget->engine()->rootContext()->setContextObject(new KLocalizedContext(d->quickWidget));

    d->quickWidget->setSource(QUrl::fromLocalFile(d->package.filePath("mainscript")));

    if (!d->quickWidget->rootObject()) {
        for (const auto &err : d->quickWidget->errors()) {
            qWarning() << err.toString();
        }
        qFatal("Fatal error while loading the icons view qml component");
    }

    const int rootImplicitWidth = d->quickWidget->rootObject()->property("implicitWidth").toInt();
    if (rootImplicitWidth != 0) {
        d->quickWidget->setMinimumWidth(rootImplicitWidth);
    } else {
        d->quickWidget->setMinimumWidth(240);
    }
    connect(d->quickWidget->rootObject(), &QQuickItem::implicitWidthChanged, this, [this]() {
        const int rootImplicitWidth = d->quickWidget->rootObject()->property("implicitWidth").toInt();
        if (rootImplicitWidth != 0) {
            d->quickWidget->setMinimumWidth(rootImplicitWidth);
        } else {
            d->quickWidget->setMinimumWidth(240);
        }
    });

    {
        d->toolbarWidget = new QQuickWidget(d->mainWidget);
        d->toolbarWidget->quickWindow()->setTitle(i18n("Icons"));
        d->toolbarWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
        qmlRegisterUncreatableType<IconsMode>(
            "org.kde.systemsettings", 1, 0, "SystemSettings", QStringLiteral("Not creatable, use the systemsettings attached property"));

        d->toolbarWidget->engine()->rootContext()->setContextProperty(QStringLiteral("systemsettings"), this);
        d->toolbarWidget->engine()->rootContext()->setContextObject(new KLocalizedContext(d->toolbarWidget));
        qWarning() << d->package.filePath("toolbar");
        d->toolbarWidget->setSource(QUrl::fromLocalFile(d->package.path() + QStringLiteral("contents/ui/Toolbar.qml")));

        if (!d->toolbarWidget->rootObject()) {
            for (const auto &err : d->toolbarWidget->errors()) {
                qWarning() << err.toString();
            }
            qFatal("Fatal error while loading the icons view qml component");
        }

        auto item = d->toolbarWidget->rootObject();

        auto update = [tb = d->toolbarWidget, item]() {
            tb->setFixedHeight(item->implicitHeight());
        };
        update();
        connect(item, &QQuickItem::implicitHeightChanged, this, update);
    }

    setHeaderHeight(d->quickWidget->rootObject()->property("headerHeight").toReal());

    connect(d->quickWidget->rootObject(), SIGNAL(focusNextRequest()), d->mainWidget, SLOT(focusNext()));
    connect(d->quickWidget->rootObject(), SIGNAL(focusPreviousRequest()), d->mainWidget, SLOT(focusPrevious()));

    d->quickWidget->installEventFilter(this);
    d->quickWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    d->bottomLayout->addWidget(d->quickWidget);
    d->moduleView->hide();
    d->bottomLayout->addWidget(d->moduleView);

    d->mainLayout->addWidget(d->toolbarWidget);
    d->mainLayout->addWidget(d->bottomWidget);

    Q_EMIT changeToolBarItems(BaseMode::NoItems);

    d->toolTipManager = new ToolTipManager(d->categorizedModel, d->quickWidget, ToolTipManager::ToolTipPosition::Right);

    d->mostUsedModel->setResultModel(new ResultModel(AllResources | Agent(QStringLiteral("org.kde.systemsettings")) | HighScoredFirst | Limit(5), this));

    if (!startupModule().isEmpty()) {
        MenuItem *item = rootItem()->descendantForModule(startupModule());
        if (item) {
            loadModule(d->model->indexForItem(item), startupModuleArgs());
        }
    }
}

void IconsMode::reloadStartupModule()
{
    if (!startupModule().isEmpty()) {
        MenuItem *item = rootItem()->descendantForModule(startupModule());
        if (item) {
            loadModule(d->model->indexForItem(item), startupModuleArgs());
        }
    }
}

bool IconsMode::eventFilter(QObject *watched, QEvent *event)
{
    // FIXME: those are all workarounds around the QQuickWidget brokeness
    if ((watched == d->quickWidget) && event->type() == QEvent::KeyPress) {
        // allow tab navigation inside the qquickwidget
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        QQuickWidget *qqw = static_cast<QQuickWidget *>(watched);
        if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
            QCoreApplication::sendEvent(qqw->quickWindow(), event);
            return true;
        }
    } else if ((watched == d->quickWidget) && event->type() == QEvent::FocusIn) {
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
        Q_EMIT widthChanged();
    } else if (watched == d->mainWidget && event->type() == QEvent::Show) {
        Q_EMIT changeToolBarItems(BaseMode::NoItems);
    }
    return BaseMode::eventFilter(watched, event);
}

void IconsMode::giveFocus()
{
    d->quickWidget->setFocus();
}

#include "IconsMode.moc"
