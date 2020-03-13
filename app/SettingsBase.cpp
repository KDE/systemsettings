/***************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <bcooksley@kde.org>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "SettingsBase.h"
#include "BaseConfig.h"
#include "systemsettings_app_debug.h"

#include <QTimer>
#include <QRadioButton>
#include <QVariantList>
#include <QLoggingCategory>
#include <QMenu>
#include <QMenuBar>
#include <QScreen>

#include <kaboutdata.h>
#include <KMessageBox>
#include <KConfigGroup>
#include <KCModuleInfo>
#include <KXMLGUIFactory>
#include <KStandardAction>
#include <KActionCollection>
#include <KServiceTypeTrader>
#include <KToolBar>
#include <kwindowconfig.h>

#include "BaseData.h"
#include "ModuleView.h"

SettingsBase::SettingsBase(BaseMode::ApplicationMode mode, QWidget * parent )
    : KXmlGuiWindow(parent),
      m_mode(mode)
{
    // Ensure delayed loading doesn't cause a crash
    activeView = nullptr;
    aboutDialog = nullptr;
    configDialog = nullptr;
    lostFound = nullptr;
    // Prepare the view area
    stackedWidget = new QStackedWidget( this );
    setCentralWidget(stackedWidget);
    setWindowFlags( windowFlags() | Qt::WindowContextHelpButtonHint );
    // Initialise search
    searchText = new KLineEdit( this );
    searchText->setClearButtonEnabled( true );
    searchText->setPlaceholderText( i18nc( "Search through a list of control modules", "Search" ) );
    searchText->setCompletionMode( KCompletion::CompletionPopup );

    if (m_mode == BaseMode::InfoCenter) {
        actionCollection()->removeAction(configureAction);
        configureAction = nullptr;
        setWindowTitle(i18n("Info Center"));
        setWindowIcon(QIcon::fromTheme(QStringLiteral("hwinfo")));
    } else {
        setWindowTitle(i18n("System Settings"));
        setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system")));
    }

    spacerWidget = new QWidget( this );
    spacerWidget->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Maximum );
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

    // on smaller or portrait-rotated screens, do not max out height and/or width
    const QSize screenSize = (QGuiApplication::primaryScreen()->availableSize()*0.9);
    const QSize targetSize = QSize(1024, 700);
    return targetSize.boundedTo(screenSize);

}

void SettingsBase::initApplication()
{
    // Prepare the menu of all modules
    if (m_mode == BaseMode::InfoCenter) {
        categories = KServiceTypeTrader::self()->query(QStringLiteral("KInfoCenterCategory"));
        modules = KServiceTypeTrader::self()->query(QStringLiteral("KCModule"), QStringLiteral("[X-KDE-ParentApp] == 'kinfocenter'"));
    } else {
        categories = KServiceTypeTrader::self()->query(QStringLiteral("SystemSettingsCategory"));
        modules = KServiceTypeTrader::self()->query(QStringLiteral("KCModule"), QStringLiteral("[X-KDE-System-Settings-Parent-Category] != ''"));
        modules += KServiceTypeTrader::self()->query(QStringLiteral("SystemSettingsExternalApp"));
    }

    rootModule = new MenuItem( true, nullptr );
    initMenuList(rootModule);

    // Handle lost+found modules...
    if (lostFound) {
        for (int i = 0; i < modules.size(); ++i) {
            const KService::Ptr entry = modules.at(i);
            MenuItem * infoItem = new MenuItem(false, lostFound);
            infoItem->setService( entry );
            qCDebug(SYSTEMSETTINGS_APP_LOG) << "Added " << entry->name();
        }
    }

    // Prepare the Base Data
    BaseData::instance()->setMenuItem( rootModule );
    BaseData::instance()->setHomeItem( homeModule );
    // Load all possible views
    const KService::List pluginObjects = KServiceTypeTrader::self()->query( QStringLiteral("SystemSettingsView") );
    const int nbPlugins = pluginObjects.count();
    for( int pluginsDone = 0; pluginsDone < nbPlugins ; ++pluginsDone ) {
        KService::Ptr activeService = pluginObjects.at( pluginsDone );
        QString error;
        BaseMode * controller = activeService->createInstance<BaseMode>(this, {m_mode}, &error);
        if( error.isEmpty() ) {
            possibleViews.insert( activeService->library(), controller );
            controller->init( activeService );
            connect(controller, &BaseMode::changeToolBarItems, this, &SettingsBase::changeToolBar);
            connect(controller, &BaseMode::actionsChanged, this, &SettingsBase::updateViewActions);
            connect(searchText, &KLineEdit::textChanged, controller, &BaseMode::searchChanged);
            connect(controller, &BaseMode::viewChanged, this, &SettingsBase::viewChange);
        } else {
            qCWarning(SYSTEMSETTINGS_APP_LOG) << QStringLiteral("View load error: ") + error;
        }
    }
    searchText->completionObject()->setIgnoreCase( true );
    searchText->completionObject()->setItems( BaseData::instance()->menuItem()->keywords() );
    changePlugin();

    // enforce minimum window size
    setMinimumSize(SettingsBase::sizeHint());
    activateWindow();
}

void SettingsBase::initToolBar()
{
    // Fill the toolbar with default actions
    // Exit is the very last action
    quitAction = actionCollection()->addAction( KStandardAction::Quit, QStringLiteral("quit_action"), this, SLOT(close()) );

    // Configure goes at the end
    configureAction = actionCollection()->addAction( KStandardAction::Preferences, QStringLiteral("configure"), this, SLOT(configShow()) );
    configureAction->setText( i18n("Configure...") );
    // Help after it
    initHelpMenu();
    configureAction->setIcon(QIcon::fromTheme(QStringLiteral("settings-configure")));

    // There's nothing to configure in info center mode
    if (m_mode == BaseMode::InfoCenter) {
        configureAction->setVisible(false);
    }

    // Then a spacer so the search line-edit is kept separate
    spacerAction = new QWidgetAction( this );
    spacerAction->setDefaultWidget(spacerWidget);
    actionCollection()->addAction( QStringLiteral("spacer"), spacerAction );
    // Finally the search line-edit
    searchAction = new QWidgetAction( this );
    searchAction->setDefaultWidget(searchText);
    connect( searchAction, SIGNAL(triggered(bool)), searchText, SLOT(setFocus()));
    actionCollection()->addAction( QStringLiteral("searchText"), searchAction );
    // Initialise the Window
    setupGUI(Save|Create,QString());
    menuBar()->hide();

    // Toolbar & Configuration
    helpActionMenu->setMenu( dynamic_cast<QMenu*>( factory()->container(QStringLiteral("help"), this) ) );
    toolBar()->setMovable(false); // We don't allow any changes
    changeToolBar( BaseMode::Search | BaseMode::Configure );
}

void SettingsBase::initHelpMenu()
{
    helpActionMenu = new KActionMenu( QIcon::fromTheme(QStringLiteral("help-contents")), i18n("Help"), this );
    helpActionMenu->setDelayed( false );
    actionCollection()->addAction( QStringLiteral("help_toolbar_menu"), helpActionMenu );
    // Add the custom actions
    aboutModuleAction = actionCollection()->addAction( KStandardAction::AboutApp, QStringLiteral("help_about_module"), this, SLOT(about()) );
    changeAboutMenu( nullptr, aboutModuleAction, i18n("About Active Module") );
    aboutViewAction = actionCollection()->addAction( KStandardAction::AboutApp, QStringLiteral("help_about_view"), this, SLOT(about()) );
}

void SettingsBase::initConfig()
{
    // Prepare dialog first
    configDialog = new KConfigDialog( this, QStringLiteral("systemsettingsconfig"), BaseConfig::self() );
    configDialog->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Add our page
    QWidget * configPage = new QWidget( configDialog );
    configWidget.setupUi(configPage);
    QString iconName = KAboutData::applicationData().programIconName();
    configDialog->addPage( configPage, i18nc("General config for System Settings", "General"), iconName );
    QVBoxLayout * configLayout = new QVBoxLayout;
    // Get the list of modules
    foreach( BaseMode * mode, possibleViews ) {
        mode->addConfiguration( configDialog );
        QRadioButton * radioButton = new QRadioButton( mode->service()->name(), configWidget.GbViewStyle );
        radioButton->setIcon( QIcon::fromTheme(mode->service()->icon()) );
        configLayout->addWidget( radioButton );
        viewSelection.addButton( radioButton, possibleViews.values().indexOf(mode) );
    }
    configWidget.GbViewStyle->setLayout( configLayout );
    configWidget.GbViewStyle->setVisible( possibleViews.count() > 1 );
    KWindowConfig::restoreWindowSize(configDialog->windowHandle(), KSharedConfig::openConfig()->group("ConfigDialog"));
    connect(configDialog, &KConfigDialog::accepted, this, &SettingsBase::configUpdated);
}


void SettingsBase::initMenuList(MenuItem * parent)
{
    // look for any categories inside this level, and recurse into them
    for (int i = 0; i < categories.size(); ++i) {
        const KService::Ptr entry = categories.at(i);
        QString parentCategory;
        QString parentCategory2;
        if (m_mode == BaseMode::InfoCenter) {
            parentCategory = entry->property(QStringLiteral("X-KDE-KInfoCenter-Parent-Category")).toString();
        } else {
            parentCategory = entry->property(QStringLiteral("X-KDE-System-Settings-Parent-Category")).toString();
            parentCategory2 = entry->property(QStringLiteral("X-KDE-System-Settings-Parent-Category-V2")).toString();
        }

        if (parentCategory == parent->category() ||
             // V2 entries must not be empty if they want to become a proper category.
             ( !parentCategory2.isEmpty() && parentCategory2 == parent->category() ) ) {
            MenuItem * menuItem = new MenuItem(true, parent);
            menuItem->setService( entry );
            if( menuItem->category() == QLatin1String("lost-and-found") ) {
                lostFound = menuItem;
                continue;
            }
            initMenuList( menuItem );
        }
    }

    KService::List removeList;

    // scan for any modules at this level and add them
    for (int i = 0; i < modules.size(); ++i) {
        const KService::Ptr entry = modules.at(i);

        QString category;
        QString category2;
        if (m_mode == BaseMode::InfoCenter) {
            category = entry->property(QStringLiteral("X-KDE-KInfoCenter-Category")).toString();
        } else {
            category = entry->property(QStringLiteral("X-KDE-System-Settings-Parent-Category")).toString();
            category2 = entry->property(QStringLiteral("X-KDE-System-Settings-Parent-Category-V2")).toString();
        }

        if( !parent->category().isEmpty() && (category == parent->category() || category2 == parent->category()) ) {
            if (!entry->noDisplay() ) {
                // Add the module info to the menu
                MenuItem * infoItem = new MenuItem(false, parent);
                infoItem->setService( entry );
                if (m_mode == BaseMode::InfoCenter && entry->pluginKeyword() == QStringLiteral("kcm-about-distro")) {
                    homeModule = infoItem;
                }
            }

            removeList.append( modules.at(i) );
        }
    }

    for (int i = 0; i < removeList.size(); ++i) {
        modules.removeOne( removeList.at(i) );
    }
    
    parent->sortChildrenByWeight();
}

void SettingsBase::configUpdated()
{
    KConfigGroup dialogConfig = KSharedConfig::openConfig()->group("ConfigDialog");
    KWindowConfig::saveWindowSize(configDialog->windowHandle(), dialogConfig);
    BaseConfig::setActiveView( possibleViews.keys().at(viewSelection.checkedId()) );

    BaseConfig::setShowToolTips( configWidget.ChTooltips->isChecked() );
    activeView->setShowToolTips( configWidget.ChTooltips->isChecked() );
    activeView->saveConfiguration();
    changePlugin();
}

void SettingsBase::configShow()
{
    // Initialise the configuration dialog if it hasn't already
    if( !configDialog ) {
        initConfig();
    }
    if( activeView && activeView->moduleView() && !activeView->moduleView()->resolveChanges() ) {
        return; // It shouldn't be triggering anyway, since the action is disabled
    }

    activeView->loadConfiguration();

    const QStringList pluginList = possibleViews.keys();
    const int configIndex = pluginList.indexOf( BaseConfig::activeView() );
    if( configIndex != -1 ) {
        viewSelection.button( configIndex )->setChecked(true);
    }
    configWidget.ChTooltips->setChecked( BaseConfig::showToolTips() );
    if( pluginList.isEmpty() ) {
        KMessageBox::error(this, i18n("System Settings was unable to find any views, and hence nothing is available to configure."), i18n("No views found"));
    } else {
        configDialog->show();
    }
}

bool SettingsBase::queryClose()
{
    bool changes = true;
    if( activeView ) {
        activeView->saveState();
        changes = activeView->moduleView()->resolveChanges();
    }
    BaseConfig::self()->save();
    return changes;
}

void SettingsBase::about()
{
    delete aboutDialog;
    aboutDialog = nullptr;

    const KAboutData * about = nullptr;
    if( sender() == aboutViewAction ) {
        about = activeView->aboutData();
    } else if( sender() == aboutModuleAction && activeView->moduleView() ) {
        about = activeView->moduleView()->aboutData();
    }

    if( about ) {
        aboutDialog = new KAboutApplicationDialog(*about, nullptr);
        aboutDialog->show();
    }
}

void SettingsBase::changePlugin()
{
    if( possibleViews.isEmpty() ) { // We should ensure we have a plugin available to choose
        KMessageBox::error(this, i18n("System Settings was unable to find any views, and hence has nothing to display."), i18n("No views found"));
        close();
        return; // Halt now!
    }

    if( activeView ) {
        activeView->saveState();
        activeView->leaveModuleView();
    }

    const QString viewToUse = m_mode == BaseMode::InfoCenter ? QStringLiteral("systemsettings_sidebar_mode") : BaseConfig::activeView();
    if( possibleViews.keys().contains(viewToUse) ) { // First the configuration entry
        activeView = possibleViews.value(viewToUse);
    }
    else { // Otherwise we activate the failsafe
        activeView = possibleViews.begin().value();
    }

    if( stackedWidget->indexOf(activeView->mainWidget()) == -1 ) {
        stackedWidget->addWidget(activeView->mainWidget());
    }

    show();

    // Handle the tooltips
    qDeleteAll( tooltipManagers );
    tooltipManagers.clear();
    if ( BaseConfig::showToolTips() ) {
        QList<QAbstractItemView*> theViews = activeView->views();
        foreach ( QAbstractItemView* view, theViews ) {
            tooltipManagers << new ToolTipManager( view );
        }
    }
    activeView->setShowToolTips( BaseConfig::showToolTips() );

    changeAboutMenu( activeView->aboutData(), aboutViewAction, i18n("About Active View") );
    viewChange(false);

    stackedWidget->setCurrentWidget(activeView->mainWidget());
    updateViewActions();

    activeView->giveFocus();
}

void SettingsBase::viewChange(bool state)
{
    KCModuleInfo * moduleInfo = activeView->moduleView()->activeModule();
    if (configureAction) {
        configureAction->setDisabled(state);
    }
    if( moduleInfo ) {
        setCaption( moduleInfo->moduleName(), state );
    } else {
        setCaption( QString(), state );
    }
    changeAboutMenu( activeView->moduleView()->aboutData(), aboutModuleAction, i18n("About Active Module") );
}

void SettingsBase::updateViewActions()
{
    guiFactory()->unplugActionList( this, QStringLiteral("viewActions") );
    guiFactory()->plugActionList( this, QStringLiteral("viewActions"), activeView->actionsList() );
}

void SettingsBase::changeToolBar( BaseMode::ToolBarItems toolbar )
{
    if( sender() != activeView ) {
        return;
    }
    guiFactory()->unplugActionList( this, QStringLiteral("configure") );
    guiFactory()->unplugActionList( this, QStringLiteral("search") );
    guiFactory()->unplugActionList( this, QStringLiteral("quit") );
    if ( BaseMode::Search & toolbar ) {
        QList<QAction*> searchBarActions;
        searchBarActions << spacerAction << searchAction;
        guiFactory()->plugActionList( this, QStringLiteral("search"), searchBarActions );
        actionCollection()->setDefaultShortcut(searchAction, QKeySequence(Qt::CTRL + Qt::Key_F));
    }
    if ( (BaseMode::Configure & toolbar) && configureAction) {
        QList<QAction*> configureBarActions;
        configureBarActions << configureAction;
        guiFactory()->plugActionList( this, QStringLiteral("configure"), configureBarActions );
    }
    if ( BaseMode::Quit & toolbar ) {
        QList<QAction*> quitBarActions;
        quitBarActions << quitAction;
        guiFactory()->plugActionList( this, QStringLiteral("quit"), quitBarActions );
    }
    if (BaseMode::NoItems & toolbar) {
        // Remove search shortcut when there's no toolbar so it doesn't
        // interfere with the built-in shortcut for the search field in the QML
        // sidebar view
        actionCollection()->setDefaultShortcut(searchAction, QKeySequence());
    }

    toolBar()->setVisible(toolbar != BaseMode::NoItems || (activeView && activeView->actionsList().count() > 0));
}

void SettingsBase::changeAboutMenu( const KAboutData * menuAbout, QAction * menuItem, const QString &fallback )
{
    if( !menuItem ) {
        return;
    }

    if( menuAbout ) {
        menuItem->setText( i18n( "About %1", menuAbout->displayName() ) );
        menuItem->setIcon( QIcon::fromTheme( menuAbout->programIconName() ) );
        menuItem->setEnabled(true);
    } else {
        menuItem->setText( fallback );
        menuItem->setIcon( QIcon::fromTheme( KAboutData::applicationData().programIconName() ) );
        menuItem->setEnabled(false);
    }
}

