/**************************************************************************
 * Copyright (C) 2009 Ben Cooksley <bcooksley@kde.org>                    *
 * Copyright (C) 2008 Mathias Soeken <msoeken@informatik.uni-bremen.de>   *
 * Copyright 2014 Sebastian Kügler <sebas@kde.org>                        *
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


#include "QuickMode.h"
#include "Category.h"
#include "host.h"
#include "kuserproxy.h"
#include "ui_configClassic.h"

#include <QDebug>
#include <QLayout>
#include <QSplitter>
//#include <QTreeView>
#include <QModelIndex>
#include <QStackedWidget>
#include <QAbstractItemModel>
#include <QQuickWidget>
#include <QQmlContext>
#include <QtQml>

#include <Plasma/Package>
#include <Plasma/PackageStructure>
#include <Plasma/PluginLoader>

#include <KAboutData>
#include <KCModuleInfo>
#include <KConfigGroup>
#include <KConfigDialog>
//#include <KGlobalSettings>

#include "host.h"
#include "MenuItem.h"
#include "MenuModel.h"
#include "ModuleView.h"
#include "CategoryList.h"
#include "MenuProxyModel.h"

K_PLUGIN_FACTORY(QuickModeFactory, registerPlugin<QuickMode>();)

const static QString defaultPackageName = QStringLiteral("org.kde.systemsettings.breeze");

class QuickMode::Private
{
public:
    Private() :
    mainWidget(0),
    gridLayout(0),
    categoriesWidget(0),
    moduleView(0)
    //stackedWidget(0)
    {}
    virtual ~Private() {
        delete aboutClassic;
    }

    QWidget *mainWidget;
    QGridLayout *gridLayout;
    QSplitter *classicWidget;
    QQuickWidget *categoriesWidget;
    Ui::ConfigClassic classicConfig;
    CategoryList *classicCategory;
    QStackedWidget *stackedWidget;
    ModuleView *moduleView;
    QModelIndex currentItem;

    MenuProxyModel *proxyModel;
    MenuModel *model;
    KAboutData *aboutClassic;
    Plasma::Package package;
};

QuickMode::QuickMode(QObject *parent, const QVariantList &)
    : BaseMode(parent), d(new Private())
{
    d->aboutClassic = new KAboutData(QStringLiteral("BreezeView"),
                                     i18n("Breeze View"),
                                     QStringLiteral("1.0"),
                                     i18n("Provides a Breeze-styled interface to settings modules."),
                                     KAboutLicense::GPL,
                                     i18n("Copyright 2014, Sebastian Kügler"));
    d->aboutClassic->addAuthor(i18n("Sebastian Kügler"), i18n("Author"), QStringLiteral("sebas@kde.org"));
    d->aboutClassic->addAuthor(i18n("Ben Cooksley"), i18n("Author"), QStringLiteral("bcooksley@kde.org"));
    d->aboutClassic->addAuthor(i18n("Mathias Soeken"), i18n("Developer"), QStringLiteral("msoeken@informatik.uni-bremen.de"));
    d->aboutClassic->setProgramIconName("applications-science");
}

QuickMode::~QuickMode()
{
    if (!d->mainWidget) {
        delete d->mainWidget;
    }
    delete d;
}

void QuickMode::loadPackage()
{
    Plasma::PackageStructure *ps = new Plasma::PackageStructure(this);
    d->package = Plasma::Package(ps);
    d->package.addFileDefinition("Categories", "ui/Categories.qml", i18n("Sidebar Script File"));
    //d->package.setRequired("Categories", false);
    d->package.addFileDefinition("MainView", "ui/MainView.qml", i18n("Main Overview"));
    d->package.setRequired("MainView", true);
    d->package.addFileDefinition("Modules", "ui/Modules.qml", i18n("Modules List Script File"));
    //d->package.setRequired("Modules", false);

    const QString packageName = config().readEntry("package", defaultPackageName);
    const QString pr = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              QStringLiteral("plasma/packages/") + packageName,
                                              QStandardPaths::LocateDirectory);
    d->package.setPath(pr);

    if (d->package.filePath("MainView").isEmpty()) {
        qWarning() << "Package " << packageName << " is not installed.";
    }
//     qDebug() << "valid / defroot?: " << d->package.isValid() << d->package.defaultPackageRoot();
//     qDebug() << "categories: " << d->package.filePath("Categories");
//     qDebug() << "modules: " << d->package.filePath("Modules");

}

void QuickMode::initEvent()
{
    loadPackage();
    // Create the model
    d->model = new MenuModel(rootItem(), this);

    // Move items that are the sole child of a category up....
    moveUp(rootItem());

    // Create the proxy model
    d->proxyModel = new MenuProxyModel(this);
    d->proxyModel->setSourceModel(d->model);
    d->proxyModel->sort(0);

    Host::self()->setModel(d->proxyModel);
    Host::self()->setQuickMode(this);

    // Register MenuItem* in the QML runtime
    qmlRegisterUncreatableType<MenuItem>("org.kde.systemsettings", 5, 0, "MenuItem", "You cannot create MenuItem objects.");
    qmlRegisterUncreatableType<Category>("org.kde.systemsettings", 5, 0, "Category", "You cannot create Category objects.");
    qmlRegisterType<KUserProxy>("org.kde.systemsettings", 5, 0, "KUser");
}

QWidget *QuickMode::mainWidget()
{
    if (!d->mainWidget) {
        initWidget();
    }
    return d->mainWidget;
}

KAboutData *QuickMode::aboutData()
{
    return d->aboutClassic;
}

ModuleView *QuickMode::moduleView() const
{
    return d->moduleView;
}

QList<QAbstractItemView *> QuickMode::views() const
{
    QList<QAbstractItemView *> theViews;
    //theViews << d->categoriesWidget;
    return theViews;
}

void QuickMode::saveState()
{
    config().writeEntry("viewLayout", d->classicWidget->sizes());
    config().sync();
}

void QuickMode::searchChanged(const QString &text)
{
    d->proxyModel->setFilterRegExp(text);
    if (d->categoriesWidget) {
        // FIXME
    }
}

void QuickMode::selectModule(const QModelIndex &selectedModule)
{
    changeModule(selectedModule);
}

void QuickMode::changeModule(const QModelIndex &activeModule)
{
    if (activeModule == d->currentItem) {
        return;
    }
    if (!d->moduleView->resolveChanges()) {
        return;
    }
    d->moduleView->closeModules();
    d->currentItem = activeModule;
    if (d->proxyModel->rowCount(activeModule) > 0) {
        //d->moduleView->hide();
        setModuleWidgetVisible(false);
        //d->classicCategory->changeModule(activeModule);
        qDebug() << "Group, I think.";
        emit viewChanged(false);
    } else {
        qDebug() << "Load Module RAISE";
        d->moduleView->loadModule(activeModule);
        setModuleWidgetVisible(true); // FIXME move to loaded
    }
}

void QuickMode::moduleLoaded()
{
    d->moduleView->show();
}

void QuickMode::initWidget()
{

    d->mainWidget = new QWidget();
    d->gridLayout = new QGridLayout(d->mainWidget);

    d->moduleView = new ModuleView(d->mainWidget);
    d->gridLayout->addWidget(d->moduleView, 1, 1);

    // Create the widget
    d->categoriesWidget = new QQuickWidget(d->mainWidget);
    d->categoriesWidget->setAutoFillBackground(false);
    d->categoriesWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    d->categoriesWidget->rootContext()->setContextProperty("host", Host::self());
    d->categoriesWidget->setSource(d->package.filePath("MainView"));
    d->gridLayout->addWidget(d->categoriesWidget, 0, 0, 2, 2);

    connect(Host::self(), &Host::moduleSelected, this, &QuickMode::selectModule);
    connect(d->moduleView, SIGNAL(moduleChanged(bool)), this, SLOT(moduleLoaded()));
    connect(d->categoriesWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(changeModule(QModelIndex)));
}

void QuickMode::setColumnWidth(int col, int colWidth)
{
    d->gridLayout->setColumnMinimumWidth(col, colWidth);
}

void QuickMode::setRowHeight(int row, int rowHeight)
{
    d->gridLayout->setRowMinimumHeight(row, rowHeight);
}


void QuickMode::leaveModuleView()
{
//     d->moduleView->closeModules();
//     //d->stackedWidget->setCurrentWidget(d->classicCategory);
//     d->moduleView->hide();
    setModuleWidgetVisible(false);
}

void QuickMode::setModuleWidgetVisible(bool vis, bool noCallback)
{
    if (vis) {
        d->moduleView->show();
        d->moduleView->raise();
    } else {
        d->moduleView->closeModules();
        //d->stackedWidget->setCurrentWidget(d->classicCategory);
        d->moduleView->hide();
    }
    if (!noCallback) {
        Host::self()->setModuleWidgetVisible(vis);
    }
}

void QuickMode::giveFocus()
{
    d->categoriesWidget->setFocus();
}

void QuickMode::addConfiguration(KConfigDialog *config)
{
    QWidget *configWidget = new QWidget(config);
    d->classicConfig.setupUi(configWidget);
    config->addPage(configWidget, i18n("Breeze View"), aboutData()->programIconName());
}

void QuickMode::loadConfiguration()
{
    d->classicConfig.CbExpand->setChecked(config().readEntry("autoExpandOneLevel", false));
}

void QuickMode::saveConfiguration()
{
    config().writeEntry("autoExpandOneLevel", d->classicConfig.CbExpand->isChecked());
}

void QuickMode::moveUp(MenuItem *item)
{
    foreach(MenuItem * child, item->children()) {
        if (child->children().count() == 1) {
            d->model->addException(child);
        }
        moveUp(child);
    }
}

#include "QuickMode.moc"
