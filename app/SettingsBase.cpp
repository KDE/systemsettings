/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *   SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
 *   SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SettingsBase.h"
#include "../core/kcmmetadatahelpers.h"
#include "BaseConfig.h"
#include "systemsettings_app_debug.h"

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
#include <KCModuleInfo>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KFileUtils>
#include <KIO/JobUiDelegateFactory>
#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginMetaData>
#include <KStandardAction>
#include <KToolBar>
#include <KWindowConfig>
#include <KXMLGUIFactory>

#include "BaseData.h"
#include "ModuleView.h"

SettingsBase::SettingsBase(BaseMode::ApplicationMode mode, QWidget *parent)
    : KXmlGuiWindow(parent)
    , m_mode(mode)
{
    // Ensure delayed loading doesn't cause a crash
    activeView = nullptr;
    aboutDialog = nullptr;
    lostFound = nullptr;
    // Prepare the view area
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    // Initialise search
    searchText = new KLineEdit(this);
    searchText->setClearButtonEnabled(true);
    searchText->setPlaceholderText(i18nc("Search through a list of control modules", "Search"));
    searchText->setCompletionMode(KCompletion::CompletionPopup);

    setProperty("_breeze_no_separator", true);

    if (m_mode == BaseMode::InfoCenter) {
        setWindowTitle(i18n("Info Center"));
        setWindowIcon(QIcon::fromTheme(QStringLiteral("hwinfo")));
    } else {
        setWindowTitle(i18n("System Settings"));
        setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system")));
    }

    spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    // Initialise the window so we don't flicker
    initToolBar();
    // We can now launch the delayed loading safely
    QTimer::singleShot(0, this, &SettingsBase::initApplication);
}

SettingsBase::~SettingsBase()
{
    delete rootModule;
}

QSize SettingsBase::sizeHint() const
{
    // Take the font size into account for the window size, as we do for UI elements
    const float fontSize = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSizeF();
    const QSize targetSize = QSize(qRound(102 * fontSize), qRound(70 * fontSize));

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
        for (const auto &metaData : qAsConst(pluginModules)) {
            auto infoItem = new MenuItem(false, lostFound);
            infoItem->setMetaData(metaData);
            qCDebug(SYSTEMSETTINGS_APP_LOG) << "Added " << metaData.pluginId();
        }
    }

    // Prepare the Base Data
    BaseData::instance()->setMenuItem(rootModule);
    BaseData::instance()->setHomeItem(homeModule);
    // Only load the current used view
    m_plugins = KPluginMetaData::findPlugins(QStringLiteral("systemsettingsview/"));
    loadCurrentView();

    searchText->completionObject()->setIgnoreCase(true);
    searchText->completionObject()->setItems(BaseData::instance()->menuItem()->keywords());
    changePlugin();

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
        switchToIconAction = actionCollection()->addAction(QStringLiteral("switchto_iconview"), this, [this] {
            BaseConfig::setActiveView(QStringLiteral("systemsettings_icon_mode"));
            changePlugin();
        });
        switchToIconAction->setText(i18n("Switch to Icon View"));
        switchToIconAction->setIcon(QIcon::fromTheme(QStringLiteral("view-list-icons")));

        switchToSidebarAction = actionCollection()->addAction(QStringLiteral("switchto_sidebar"), this, [this] {
            BaseConfig::setActiveView(QStringLiteral("systemsettings_sidebar_mode"));
            changePlugin();
        });
        switchToSidebarAction->setText(i18n("Switch to Sidebar View"));
        switchToSidebarAction->setIcon(QIcon::fromTheme(QStringLiteral("view-sidetree")));

        highlightChangesAction = actionCollection()->addAction(QStringLiteral("highlight_changes"), this, [this] {
            if (activeView) {
                activeView->toggleDefaultsIndicatorsVisibility();
            }
        });
        highlightChangesAction->setCheckable(true);
        highlightChangesAction->setText(i18n("Highlight Changed Settings"));
        highlightChangesAction->setIcon(QIcon::fromTheme(QStringLiteral("draw-highlight")));
    }

    reportPageSpecificBugAction = actionCollection()->addAction(QStringLiteral("report_bug_in_current_module"), this, [=] {
        auto job = new KIO::OpenUrlJob(QUrl(activeView->moduleView()->activeModuleMetadata().bugReportUrl()));
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
        job->start();
    });
    reportPageSpecificBugAction->setText(i18n("Report a Bug in the Current Page…"));
    reportPageSpecificBugAction->setIcon(QIcon::fromTheme(QStringLiteral("tools-report-bug")));

    // Help after it
    initHelpMenu();

    // Then a spacer so the search line-edit is kept separate
    spacerAction = new QWidgetAction(this);
    spacerAction->setDefaultWidget(spacerWidget);
    actionCollection()->addAction(QStringLiteral("spacer"), spacerAction);
    // Finally the search line-edit
    searchAction = new QWidgetAction(this);
    searchAction->setDefaultWidget(searchText);
    connect(searchAction, &QAction::triggered, searchText, QOverload<>::of(&KLineEdit::setFocus));
    actionCollection()->addAction(QStringLiteral("searchText"), searchAction);
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
    helpActionMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("help-contents")), i18n("Help"), this);
    helpActionMenu->setPopupMode(QToolButton::InstantPopup);
    actionCollection()->addAction(QStringLiteral("help_toolbar_menu"), helpActionMenu);
    // Add the custom actions
    aboutViewAction = actionCollection()->addAction(KStandardAction::AboutApp, QStringLiteral("help_about_view"), this, &SettingsBase::about);
}

void SettingsBase::initMenuList(MenuItem *parent)
{
    // look for any categories inside this level, and recurse into them
    for (const QString &category : qAsConst(categories)) {
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
            MenuItem *menuItem = new MenuItem(true, parent);
            menuItem->setCategoryConfig(file);
            if (entry.readEntry("X-KDE-System-Settings-Category") == QLatin1String("lost-and-found")) {
                lostFound = menuItem;
                continue;
            }
            initMenuList(menuItem);
        }
    }

    // scan for any modules at this level and add them
    for (const auto &metaData : qAsConst(pluginModules)) {
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
                MenuItem *infoItem = new MenuItem(false, parent);
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

BaseMode *SettingsBase::loadCurrentView()
{
    const QString viewToUse = m_mode == BaseMode::InfoCenter ? QStringLiteral("systemsettings_sidebar_mode") : BaseConfig::activeView();

    const auto pluginIt = std::find_if(m_plugins.cbegin(), m_plugins.cend(), [&viewToUse](const KPluginMetaData &plugin) {
        return viewToUse.contains(plugin.pluginId());
    });

    if (pluginIt == m_plugins.cend()) {
        return nullptr;
    }

    const auto controllerResult = KPluginFactory::instantiatePlugin<BaseMode>(*pluginIt, this, {m_mode, m_startupModule, m_startupModuleArgs});
    if (!controllerResult) {
        qCWarning(SYSTEMSETTINGS_APP_LOG) << "Error loading plugin" << controllerResult.errorText;
        return nullptr;
    }

    const auto controller = controllerResult.plugin;
    m_loadedViews.insert(viewToUse, controller);
    controller->init(*pluginIt);
    connect(controller, &BaseMode::changeToolBarItems, this, &SettingsBase::changeToolBar);
    connect(controller, &BaseMode::actionsChanged, this, &SettingsBase::updateViewActions);
    connect(searchText, &KLineEdit::textChanged, controller, &BaseMode::searchChanged);
    connect(controller, &BaseMode::viewChanged, this, &SettingsBase::viewChange);

    return controller;
}

bool SettingsBase::queryClose()
{
    bool changes = true;
    if (activeView) {
        activeView->saveState();
        changes = activeView->moduleView()->resolveChanges();
    }
    BaseConfig::self()->save();
    return changes;
}

void SettingsBase::setStartupModule(const QString &startupModule)
{
    m_startupModule = startupModule;

    if (activeView) {
        activeView->setStartupModule(startupModule);
    }
}

void SettingsBase::setStartupModuleArgs(const QStringList &startupModuleArgs)
{
    m_startupModuleArgs = startupModuleArgs;

    if (activeView) {
        activeView->setStartupModuleArgs(startupModuleArgs);
    }
}

void SettingsBase::reloadStartupModule()
{
    if (activeView) {
        activeView->reloadStartupModule();
    }
}

void SettingsBase::about()
{
    delete aboutDialog;
    aboutDialog = nullptr;

    const KAboutData *about = nullptr;
    if (sender() == aboutViewAction) {
        about = activeView->aboutData();
    }

    if (about) {
        aboutDialog = new KAboutApplicationDialog(*about, nullptr);
        aboutDialog->show();
    }
}

void SettingsBase::changePlugin()
{
    if (m_plugins.empty()) { // We should ensure we have a plugin available to choose
        KMessageBox::error(this, i18n("System Settings was unable to find any views, and hence has nothing to display."), i18n("No views found"));
        close();
        return; // Halt now!
    }

    // Don't let the user wait for nothing until the QML component is loaded.
    show();

    if (activeView) {
        activeView->saveState();
        activeView->leaveModuleView();
    }

    const QString viewToUse = m_mode == BaseMode::InfoCenter ? QStringLiteral("systemsettings_sidebar_mode") : BaseConfig::activeView();
    const auto it = m_loadedViews.constFind(viewToUse);
    if (it != m_loadedViews.cend()) {
        // First the configuration entry
        activeView = *it;
    } else if (auto *view = loadCurrentView()) {
        activeView = view;
    } else if (!m_loadedViews.empty()) { // Otherwise we activate the failsafe
        qCWarning(SYSTEMSETTINGS_APP_LOG) << "System Settings was unable to load" << viewToUse;
        activeView = m_loadedViews.cbegin().value();
    } else {
        // Current view is missing on startup, try to load alternate view.
        qCWarning(SYSTEMSETTINGS_APP_LOG) << "System Settings was unable to load" << viewToUse;
        if (viewToUse == QStringLiteral("systemsettings_icon_mode")) {
            BaseConfig::setActiveView(QStringLiteral("systemsettings_sidebar_mode"));
        } else if (m_mode != BaseMode::InfoCenter) {
            BaseConfig::setActiveView(QStringLiteral("systemsettings_icon_mode"));
        }

        if (auto *view = loadCurrentView()) {
            activeView = view;
            activeView->saveState();
        } else {
            qCWarning(SYSTEMSETTINGS_APP_LOG) << "System Settings was unable to load any views, and hence has nothing to display.";
            close();
            return; // Halt now!
        }
    }

    if (stackedWidget->indexOf(activeView->mainWidget()) == -1) {
        stackedWidget->addWidget(activeView->mainWidget());
    }

    // Handle the tooltips
    qDeleteAll(tooltipManagers);
    tooltipManagers.clear();
    const QList<QAbstractItemView *> theViews = activeView->views();
    for (QAbstractItemView *view : theViews) {
        tooltipManagers << new ToolTipManager(view);
    }

    if (highlightChangesAction) {
        highlightChangesAction->setChecked(activeView->defaultsIndicatorsVisible());
    }

    changeAboutMenu(activeView->aboutData(), aboutViewAction, i18n("About Active View"));
    viewChange(false);

    stackedWidget->setCurrentWidget(activeView->mainWidget());
    updateViewActions();

    activeView->giveFocus();

    // Update visibility of the "report a bug on this page" and "report bug in general"
    // actions based on whether the current page has a bug report URL set
    auto reportGeneralBugAction = actionCollection()->action(QStringLiteral("help_report_bug"));
    reportGeneralBugAction->setVisible(false);
    auto moduleView = activeView->moduleView();
    connect(moduleView, &ModuleView::moduleChanged, this, [=] {
        reportPageSpecificBugAction->setVisible(!moduleView->activeModuleMetadata().bugReportUrl().isEmpty());
        reportGeneralBugAction->setVisible(!reportPageSpecificBugAction->isVisible());
    });
}

void SettingsBase::viewChange(bool state)
{
    setCaption(activeView->moduleView()->activeModuleName(), state);
}

void SettingsBase::updateViewActions()
{
    guiFactory()->unplugActionList(this, QStringLiteral("viewActions"));
    guiFactory()->plugActionList(this, QStringLiteral("viewActions"), activeView->actionsList());
}

void SettingsBase::changeToolBar(BaseMode::ToolBarItems toolbar)
{
    if (sender() != activeView) {
        return;
    }
    guiFactory()->unplugActionList(this, QStringLiteral("configure"));
    guiFactory()->unplugActionList(this, QStringLiteral("search"));
    guiFactory()->unplugActionList(this, QStringLiteral("quit"));
    if (BaseMode::Search & toolbar) {
        QList<QAction *> searchBarActions;
        searchBarActions << spacerAction << searchAction;
        guiFactory()->plugActionList(this, QStringLiteral("search"), searchBarActions);
        actionCollection()->setDefaultShortcut(searchAction, QKeySequence(Qt::CTRL | Qt::Key_F));
    }
    if ((BaseMode::Configure & toolbar) && switchToSidebarAction) {
        QList<QAction *> configureBarActions;
        configureBarActions << switchToSidebarAction;
        guiFactory()->plugActionList(this, QStringLiteral("configure"), configureBarActions);
    }
    if (BaseMode::Quit & toolbar) {
        QList<QAction *> quitBarActions;
        quitBarActions << quitAction;
        guiFactory()->plugActionList(this, QStringLiteral("quit"), quitBarActions);
    }
    if (BaseMode::NoItems & toolbar) {
        // Remove search shortcut when there's no toolbar so it doesn't
        // interfere with the built-in shortcut for the search field in the QML
        // sidebar view
        actionCollection()->setDefaultShortcut(searchAction, QKeySequence());
    }

    toolBar()->setVisible(toolbar != BaseMode::NoItems || (activeView && activeView->actionsList().count() > 0));
}

void SettingsBase::changeAboutMenu(const KAboutData *menuAbout, QAction *menuItem, const QString &fallback)
{
    if (!menuItem) {
        return;
    }

    if (menuAbout) {
        menuItem->setText(i18n("About %1", menuAbout->displayName()));
        menuItem->setIcon(QGuiApplication::windowIcon());
        menuItem->setEnabled(true);
    } else {
        menuItem->setText(fallback);
        menuItem->setIcon(QGuiApplication::windowIcon());
        menuItem->setEnabled(false);
    }
}

void SettingsBase::slotGeometryChanged()
{
    setMinimumSize(SettingsBase::sizeHint());
}
