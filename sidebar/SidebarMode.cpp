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

class SubcategoryModel : public QStandardItemModel
{
public:
    explicit SubcategoryModel(QAbstractItemModel *parentModel, QObject *parent = nullptr)
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
    explicit MostUsedModel(QObject *parent = nullptr)
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

    QHash<int, QByteArray> roleNames() const override
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

    ToolTipManager *toolTipManager = nullptr;
    ToolTipManager *subCategoryToolTipManager = nullptr;
    QQuickWidget * quickWidget = nullptr;
    KPackage::Package package;
    SubcategoryModel * subCategoryModel = nullptr;
    MostUsedModel * mostUsedModel = nullptr;
    FocusHackWidget * mainWidget = nullptr;
    QQuickWidget * placeHolderWidget = nullptr;
    QHBoxLayout * mainLayout = nullptr;
    KDeclarative::KDeclarative kdeclarative;
    MenuProxyModel * categorizedModel = nullptr;
    MenuProxyModel * searchModel = nullptr;
    KAboutData * aboutIcon = nullptr;
    ModuleView * moduleView = nullptr;
    KActionCollection *collection = nullptr;
    QPersistentModelIndex activeCategoryIndex;
    int activeCategory;
    int activeSubCategory;
};

SidebarMode::SidebarMode( QObject *parent, const QVariantList& )
    : BaseMode( parent )
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
    d->mainWidget = new FocusHackWidget();
    d->mainWidget->installEventFilter(this);
    d->mainLayout = new QHBoxLayout(d->mainWidget);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->moduleView = new ModuleView( d->mainWidget );
    connect( d->moduleView, &ModuleView::moduleChanged, this, &SidebarMode::moduleLoaded );
    d->quickWidget = nullptr;
    moduleView()->setFaceType(KPageView::Plain);
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

void SidebarMode::requestToolTip(int index, const QRectF &rect)
{
    if (showToolTips()) {
        d->toolTipManager->requestToolTip(d->searchModel->index(index, 0), rect.toRect());
    }
}

void SidebarMode::requestSubCategoryToolTip(int index, const QRectF &rect)
{
    if (showToolTips()) {
        d->subCategoryToolTipManager->requestToolTip(d->subCategoryModel->index(index, 0), rect.toRect());
    }
}

void SidebarMode::hideToolTip()
{
    d->toolTipManager->hideToolTip();
}

void SidebarMode::hideSubCategoryToolTip()
{
    d->subCategoryToolTipManager->hideToolTip();
}

void SidebarMode::loadMostUsed(int index)
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
    const int newCategoryRow = d->searchModel->mapToSource(idx).row();

    if (d->activeCategory == newCategoryRow) {
        return;
    }
    if( !d->moduleView->resolveChanges() ) {
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

    if( !d->moduleView->resolveChanges() ) {
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
    d->quickWidget->quickWindow()->setTitle(i18n("Sidebar"));
    d->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

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

    d->toolTipManager = new ToolTipManager(d->searchModel, d->quickWidget);
    d->subCategoryToolTipManager = new ToolTipManager(d->subCategoryModel, d->quickWidget);

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

    d->mostUsedModel->setResultModel(new ResultModel( AllResources | Agent(QStringLiteral("org.kde.systemsettings")) | HighScoredFirst | Limit(5), this));
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
