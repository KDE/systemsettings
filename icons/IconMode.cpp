/*
 * SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "IconMode.h"
#include "CategorizedView.h"
#include "CategoryDrawer.h"

#include "MenuItem.h"
#include "MenuModel.h"
#include "MenuProxyModel.h"
#include "ModuleView.h"

#include <QStackedWidget>

#include <KAboutData>
#include <KFileItemDelegate>
#include <KLocalizedString>
#include <KStandardAction>
#include <QAction>

K_PLUGIN_CLASS_WITH_JSON(IconMode, "settings-icon-view.json")

class IconMode::Private
{
public:
    Private()
    {
    }

    virtual ~Private()
    {
        delete aboutIcon;
    }

    KCategoryDrawer *categoryDrawer = nullptr;
    KCategorizedView *categoryView = nullptr;
    QStackedWidget *mainWidget = nullptr;
    MenuModel *model = nullptr;
    MenuProxyModel *proxyModel = nullptr;
    KAboutData *aboutIcon = nullptr;
    ModuleView *moduleView = nullptr;
    QAction *backAction = nullptr;
};

IconMode::IconMode(QObject *parent, const QVariantList &args)
    : BaseMode(parent, args)
    , d(new Private())
{
    d->aboutIcon = new KAboutData(QStringLiteral("IconView"),
                                  i18n("Icon View"),
                                  QStringLiteral("1.0"),
                                  i18n("Provides a categorized icons view of control modules."),
                                  KAboutLicense::GPL,
                                  i18n("(c) 2009, Ben Cooksley"));
    d->aboutIcon->addAuthor(i18n("Ben Cooksley"), i18n("Author"), QStringLiteral("bcooksley@kde.org"));
    d->aboutIcon->addAuthor(i18n("Mathias Soeken"), i18n("Developer"), QStringLiteral("msoeken@informatik.uni-bremen.de"));

    d->backAction = KStandardAction::back(this, SLOT(backToOverview()), this);
    d->backAction->setText(i18n("All Settings"));
    d->backAction->setToolTip(i18n("Keyboard Shortcut: %1", d->backAction->shortcut().toString(QKeySequence::NativeText)));
    d->backAction->setEnabled(false);
    actionsList() << d->backAction;
}

IconMode::~IconMode()
{
    delete d;
}

KAboutData *IconMode::aboutData()
{
    return d->aboutIcon;
}

ModuleView *IconMode::moduleView() const
{
    return d->moduleView;
}

QWidget *IconMode::mainWidget()
{
    if (!d->categoryView) {
        initWidget();
    }
    return d->mainWidget;
}

QList<QAbstractItemView *> IconMode::views() const
{
    QList<QAbstractItemView *> list;
    list.append(d->categoryView);
    return list;
}

void IconMode::initEvent()
{
    d->model = new MenuModel(rootItem(), this);
    foreach (MenuItem *child, rootItem()->children()) {
        d->model->addException(child);
    }

    d->proxyModel = new MenuProxyModel(this);
    d->proxyModel->setCategorizedModel(true);
    d->proxyModel->setSourceModel(d->model);
    d->proxyModel->sort(0);

    d->mainWidget = new QStackedWidget();
    d->moduleView = new ModuleView(d->mainWidget);
    connect(d->moduleView, &ModuleView::moduleChanged, this, &IconMode::moduleLoaded);
    connect(d->moduleView, &ModuleView::closeRequest, this, &IconMode::backToOverview);
    d->categoryView = nullptr;
}

void IconMode::searchChanged(const QString &text)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    d->proxyModel->setFilterRegExp(text);
#else
    d->proxyModel->setFilterRegularExpression(text);
#endif
    if (d->categoryView) {
        QAbstractItemModel *model = d->categoryView->model();
        const int column = d->categoryView->modelColumn();
        const QModelIndex root = d->categoryView->rootIndex();
        for (int i = 0; i < model->rowCount(); ++i) {
            const QModelIndex index = model->index(i, column, root);
            if (model->flags(index) & Qt::ItemIsEnabled) {
                d->categoryView->scrollTo(index);
                break;
            }
        }
    }
}

void IconMode::changeModule(const QModelIndex &activeModule)
{
    // Already loaded?
    MenuItem *item = activeModule.data(Qt::UserRole).value<MenuItem *>();
    if (d->moduleView->activeModuleName() == item->name())
        return;

    changeModuleWithArgs(activeModule, QStringList());
}

void IconMode::changeModuleWithArgs(const QModelIndex &activeModule, const QStringList &args)
{
    // Always reopen because args might have changed
    d->moduleView->closeModules();
    d->mainWidget->setCurrentWidget(d->moduleView);

    const bool openCategoryFirst = activeModule.parent().isValid();

    // avoid double titles by setting the right face type before loading the module
    if (openCategoryFirst || d->categoryView->model()->rowCount(activeModule) > 1) {
        d->moduleView->setFaceType(KPageView::List);
    } else {
        d->moduleView->setFaceType(KPageView::Plain);
    }

    if (openCategoryFirst) {
        d->moduleView->loadModule(activeModule.parent(), {});
    }
    d->moduleView->loadModule(activeModule, args);

    for (int done = 1 /* starting from 1 to avoid double global theme */; activeModule.model()->rowCount(activeModule) > done; done = 1 + done) {
        QModelIndex subpageItem = activeModule.model()->index(done, 0, activeModule);
        d->moduleView->loadModule(subpageItem, args);
    }
}

void IconMode::moduleLoaded()
{
    d->backAction->setEnabled(true);
    Q_EMIT changeToolBarItems(BaseMode::NoItems);
}

void IconMode::backToOverview()
{
    if (d->moduleView->resolveChanges()) {
        d->mainWidget->setCurrentWidget(d->categoryView);
        d->moduleView->closeModules();
        d->backAction->setEnabled(false);
        Q_EMIT changeToolBarItems(BaseMode::Search | BaseMode::Configure | BaseMode::Quit);
        Q_EMIT viewChanged(false);
    }
}

void IconMode::initWidget()
{
    // Create the widget
    d->categoryView = new CategorizedView(d->mainWidget);
    d->categoryDrawer = new CategoryDrawer(d->categoryView);

    d->categoryView->setSelectionMode(QAbstractItemView::SingleSelection);
    // PORT QT5 d->categoryView->setSpacing( KDialog::spacingHint() );
    d->categoryView->setCategoryDrawer(d->categoryDrawer);
    d->categoryView->setViewMode(QListView::IconMode);
    d->categoryView->setMouseTracking(true);
    d->categoryView->viewport()->setAttribute(Qt::WA_Hover);

    KFileItemDelegate *delegate = new KFileItemDelegate(d->categoryView);
    delegate->setWrapMode(QTextOption::WordWrap);
    d->categoryView->setItemDelegate(delegate);

    d->categoryView->setFrameShape(QFrame::NoFrame);
    d->categoryView->setModel(d->proxyModel);
    connect(d->categoryView, &QAbstractItemView::activated, this, &IconMode::changeModule);

    d->mainWidget->addWidget(d->categoryView);
    d->mainWidget->addWidget(d->moduleView);
    d->mainWidget->setCurrentWidget(d->categoryView);
    Q_EMIT changeToolBarItems(BaseMode::Search | BaseMode::Configure | BaseMode::Quit);
    d->mainWidget->installEventFilter(this);

    if (!startupModule().isEmpty()) {
        MenuItem *item = rootItem()->descendantForModule(startupModule());
        if (item) {
            changeModuleWithArgs(d->proxyModel->mapFromSource(d->model->indexForItem(item)), startupModuleArgs());
        }
    }
}

void IconMode::reloadStartupModule()
{
    if (!startupModule().isEmpty()) {
        MenuItem *item = rootItem()->descendantForModule(startupModule());
        if (item) {
            changeModuleWithArgs(d->proxyModel->mapFromSource(d->model->indexForItem(item)), startupModuleArgs());
        }
    }
}

bool IconMode::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->mainWidget && event->type() == QEvent::Show) {
        Q_EMIT changeToolBarItems(BaseMode::Search | BaseMode::Configure | BaseMode::Quit);
    }
    return BaseMode::eventFilter(watched, event);
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
