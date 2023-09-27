/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *   SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
 *   SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SettingsBase.h"
#include "../core/kcmmetadatahelpers.h"
#include "../systemsettings_app_debug.h"
#include "sidebar/SidebarMode.h"

#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QMenu>
#include <QMenuBar>
#include <QRadioButton>
#include <QScreen>
#include <QTimer>
#include <QVariantList>
#include <QtGlobal>

#include <KAboutData>
#include <KActionCollection>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KFileUtils>
#include <KIO/JobUiDelegateFactory>
#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardAction>
#include <KToolBar>
#include <KWindowConfig>
#include <KXMLGUIFactory>

#include "BaseData.h"
#include "ModuleView.h"

SettingsBase::SettingsBase(BaseMode::ApplicationMode mode, const QString &startupModule, const QStringList &startupModuleArgs, QWidget *parent)
    : KXmlGuiWindow(parent)
    , m_mode(mode)
    , m_startupModule(startupModule)
    , m_startupModuleArgs(startupModuleArgs)
{
    // Prepare the view area
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    setProperty("_breeze_no_separator", true);

    if (m_mode == BaseMode::InfoCenter) {
        setWindowTitle(i18nd("systemsettings", "Info Center"));
        setWindowIcon(QIcon::fromTheme(QStringLiteral("hwinfo")));
    } else {
        setWindowTitle(i18nd("systemsettings", "System Settings"));
        setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system")));
    }

    spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    // Initialise the window so we don't flicker
    initToolBar();
    // We can now launch the delayed loading safely
    initApplication();
}

SettingsBase::~SettingsBase()
{
    delete rootModule;
}

QSize SettingsBase::sizeHint() const
{
    // Take the font size into account for the window size, as we do for UI elements
    const float fontSize = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSizeF();
    const QSize targetSize = QSize(qRound(93 * fontSize), qRound(65 * fontSize));

    // on smaller or portrait-rotated screens, do not max out height and/or width
    const QSize screenSize = (QGuiApplication::primaryScreen()->availableSize() * 0.9);
    return targetSize.boundedTo(screenSize);
}

void SettingsBase::initApplication()
{
    // Prepare the menu of all modules
    auto source = m_mode == BaseMode::InfoCenter ? MetaDataSource::KInfoCenter : MetaDataSource::SystemSettings;
    pluginModules = findKCMsMetaData(source) << findExternalKCMModules(source);

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, QStringLiteral("categories"), QStandardPaths::LocateDirectory);
    categories = KFileUtils::findAllUniqueFiles(dirs, QStringList(QStringLiteral("*.desktop")));

    rootModule = new MenuItem(true, nullptr);
    initMenuList(rootModule);

    // Handle lost+found modules...
    if (lostFound) {
        for (const auto &metaData : std::as_const(pluginModules)) {
            auto infoItem = new MenuItem(false, lostFound);
            infoItem->setMetaData(metaData);
            qCDebug(SYSTEMSETTINGS_APP_LOG) << "Added " << metaData.pluginId();
        }
    }

    // Prepare the Base Data
    BaseData::instance()->setMenuItem(rootModule);
    BaseData::instance()->setHomeItem(homeModule);
    loadCurrentView();

    // enforce minimum window size
    setMinimumSize(SettingsBase::sizeHint());
    activateWindow();

    // Change size limit after screen resolution is changed
    m_screen = qGuiApp->primaryScreen();
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, [this](QScreen *screen) {
        if (m_screen) {
            disconnect(m_screen, &QScreen::geometryChanged, this, &SettingsBase::slotGeometryChanged);
        }
        m_screen = screen;
        slotGeometryChanged();
        connect(m_screen, &QScreen::geometryChanged, this, &SettingsBase::slotGeometryChanged);
    });
    connect(m_screen, &QScreen::geometryChanged, this, &SettingsBase::slotGeometryChanged);
}

void SettingsBase::initToolBar()
{
    // Fill the toolbar with default actions
    // Exit is the very last action
    quitAction = actionCollection()->addAction(KStandardAction::Quit, QStringLiteral("quit_action"), this, &QWidget::close);

    if (m_mode == BaseMode::SystemSettings) {
        highlightChangesAction = actionCollection()->addAction(QStringLiteral("highlight_changes"), this, [this] {
            view->toggleDefaultsIndicatorsVisibility();
        });
        highlightChangesAction->setCheckable(true);
        highlightChangesAction->setText(i18nd("systemsettings", "Highlight Changed Settings"));
        highlightChangesAction->setIcon(QIcon::fromTheme(QStringLiteral("draw-highlight")));
    }

    reportPageSpecificBugAction = actionCollection()->addAction(QStringLiteral("report_bug_in_current_module"), this, [=] {
        const QString bugReportUrlString =
            view->moduleView()->activeModuleMetadata().bugReportUrl() + QStringLiteral("&version=") + QGuiApplication::applicationVersion();
        auto job = new KIO::OpenUrlJob(QUrl(bugReportUrlString));
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
        job->start();
    });
    reportPageSpecificBugAction->setText(i18nd("systemsettings", "Report a Bug in the Current Page…"));
    reportPageSpecificBugAction->setIcon(QIcon::fromTheme(QStringLiteral("tools-report-bug")));

    // Help after it
    initHelpMenu();

    // Then a spacer so the search line-edit is kept separate
    spacerAction = new QWidgetAction(this);
    spacerAction->setDefaultWidget(spacerWidget);
    actionCollection()->addAction(QStringLiteral("spacer"), spacerAction);
    // Initialise the Window
    setupGUI(Save | Create, QString());
    menuBar()->hide();

    // Toolbar & Configuration
    helpActionMenu->setMenu(dynamic_cast<QMenu *>(factory()->container(QStringLiteral("help"), this)));
    toolBar()->setMovable(false); // We don't allow any changes
    changeToolBar(BaseMode::Search | BaseMode::Configure);
}

void SettingsBase::initHelpMenu()
{
    helpActionMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("help-contents")), i18nd("systemsettings", "Help"), this);
    helpActionMenu->setPopupMode(QToolButton::InstantPopup);
    actionCollection()->addAction(QStringLiteral("help_toolbar_menu"), helpActionMenu);
    // Add the custom actions
    aboutViewAction = actionCollection()->addAction(KStandardAction::AboutApp, QStringLiteral("help_about_view"), this, &SettingsBase::about);
}

void SettingsBase::initMenuList(MenuItem *parent)
{
    // look for any categories inside this level, and recurse into them
    for (const QString &category : std::as_const(categories)) {
        const KDesktopFile file(category);
        const KConfigGroup entry = file.desktopGroup();
        QString parentCategory;
        QString parentCategory2;
        if (m_mode == BaseMode::InfoCenter) {
            parentCategory = entry.readEntry("X-KDE-KInfoCenter-Parent-Category");
        } else {
            parentCategory = entry.readEntry("X-KDE-System-Settings-Parent-Category");
            parentCategory2 = entry.readEntry("X-KDE-System-Settings-Parent-Category-V2");
        }

        if (parentCategory == parent->category() ||
            // V2 entries must not be empty if they want to become a proper category.
            (!parentCategory2.isEmpty() && parentCategory2 == parent->category())) {
            auto menuItem = new MenuItem(true, parent);
            menuItem->setCategoryConfig(file);
            if (entry.readEntry("X-KDE-System-Settings-Category") == QLatin1String("lost-and-found")) {
                lostFound = menuItem;
                continue;
            }
            initMenuList(menuItem);
        }
    }

    // scan for any modules at this level and add them
    for (const auto &metaData : std::as_const(pluginModules)) {
        QString category;
        QString categoryv2;
        if (m_mode == BaseMode::InfoCenter) {
            category = metaData.value(QStringLiteral("X-KDE-KInfoCenter-Category"));
        } else {
            category = metaData.value(QStringLiteral("X-KDE-System-Settings-Parent-Category"));
            categoryv2 = metaData.value(QStringLiteral("X-KDE-System-Settings-Parent-Category-V2"));
        }
        const QString parentCategoryKcm = parent->systemsettingsCategoryModule();
        bool isCategoryOwner = false;

        if (!parentCategoryKcm.isEmpty() && parentCategoryKcm == metaData.pluginId()) {
            parent->setMetaData(metaData);
            isCategoryOwner = true;
        }

        if (!parent->category().isEmpty() && (category == parent->category() || categoryv2 == parent->category())) {
            if (!metaData.isHidden()) {
                // Add the module info to the menu
                auto infoItem = new MenuItem(false, parent);
                infoItem->setMetaData(metaData);
                infoItem->setCategoryOwner(isCategoryOwner);

                if (m_mode == BaseMode::InfoCenter && metaData.pluginId() == QStringLiteral("kcm_about-distro")) {
                    homeModule = infoItem;
                } else if (m_mode == BaseMode::SystemSettings && metaData.pluginId() == QStringLiteral("kcm_landingpage")) {
                    homeModule = infoItem;
                }
            }
        }
    }

    parent->sortChildrenByWeight();
}

bool SettingsBase::queryClose()
{
    bool changes = true;
    view->saveState();
    changes = view->moduleView()->resolveChanges();
    return changes;
}

void SettingsBase::setStartupModule(const QString &startupModule)
{
    m_startupModule = startupModule;
    view->setStartupModule(startupModule);
}

void SettingsBase::setStartupModuleArgs(const QStringList &startupModuleArgs)
{
    m_startupModuleArgs = startupModuleArgs;

    view->setStartupModuleArgs(startupModuleArgs);
}

void SettingsBase::reloadStartupModule()
{
    view->reloadStartupModule();
}

void SettingsBase::about()
{
    delete aboutDialog;
    aboutDialog = nullptr;

    aboutDialog = new KAboutApplicationDialog(KAboutData::applicationData(), nullptr);
    aboutDialog->show();
}

void SettingsBase::loadCurrentView()
{
    view = new SidebarMode(this, {m_mode, m_startupModule, m_startupModuleArgs});
    connect(view, &BaseMode::changeToolBarItems, this, &SettingsBase::changeToolBar);
    connect(view, &BaseMode::actionsChanged, this, &SettingsBase::updateViewActions);
    connect(view, &BaseMode::viewChanged, this, &SettingsBase::viewChange);
    view->saveState();

    if (stackedWidget->indexOf(view->mainWidget()) == -1) {
        stackedWidget->addWidget(view->mainWidget());
    }

    // Handle the tooltips
    qDeleteAll(tooltipManagers);
    tooltipManagers.clear();
    const QList<QAbstractItemView *> theViews = view->views();
    for (QAbstractItemView *view : theViews) {
        tooltipManagers << new ToolTipManager(view);
    }

    if (highlightChangesAction) {
        highlightChangesAction->setChecked(view->defaultsIndicatorsVisible());
    }

    viewChange(false);

    stackedWidget->setCurrentWidget(view->mainWidget());
    updateViewActions();

    view->giveFocus();

    // Update visibility of the "report a bug on this page" and "report bug in general"
    // actions based on whether the current page has a bug report URL set
    auto reportGeneralBugAction = actionCollection()->action(QStringLiteral("help_report_bug"));
    if (reportGeneralBugAction) {
        reportGeneralBugAction->setVisible(false);
    }
    auto moduleView = view->moduleView();
    connect(moduleView, &ModuleView::moduleChanged, this, [=] {
        reportPageSpecificBugAction->setVisible(!moduleView->activeModuleMetadata().bugReportUrl().isEmpty());
        if (reportGeneralBugAction) {
            reportGeneralBugAction->setVisible(!reportPageSpecificBugAction->isVisible());
        }
    });

    show();
}

void SettingsBase::viewChange(bool state)
{
    setCaption(view->moduleView()->activeModuleName(), state);
}

void SettingsBase::updateViewActions()
{
    guiFactory()->unplugActionList(this, QStringLiteral("viewActions"));
    guiFactory()->plugActionList(this, QStringLiteral("viewActions"), view->actionsList());
}

void SettingsBase::changeToolBar(BaseMode::ToolBarItems toolbar)
{
    if (sender() != view) {
        return;
    }
    guiFactory()->unplugActionList(this, QStringLiteral("configure"));
    guiFactory()->unplugActionList(this, QStringLiteral("search"));
    guiFactory()->unplugActionList(this, QStringLiteral("quit"));
    if (BaseMode::Search & toolbar) {
        QList<QAction *> searchBarActions;
        searchBarActions << spacerAction;
        guiFactory()->plugActionList(this, QStringLiteral("search"), searchBarActions);
    }
    if (BaseMode::Quit & toolbar) {
        QList<QAction *> quitBarActions;
        quitBarActions << quitAction;
        guiFactory()->plugActionList(this, QStringLiteral("quit"), quitBarActions);
    }

    toolBar()->setVisible(toolbar != BaseMode::NoItems || (view->actionsList().count() > 0));
}

void SettingsBase::slotGeometryChanged()
{
    setMinimumSize(SettingsBase::sizeHint());
}

#include "moc_SettingsBase.cpp"
