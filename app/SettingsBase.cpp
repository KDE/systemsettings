/***************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <ben@eclipse.endoftheinternet.org> *
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

#include <QTimer>
#include <QVariantList>

#include <KDebug>
#include <KConfig>
#include <KMenuBar>
#include <KAboutData>
#include <KMessageBox>
#include <KCModuleInfo>
#include <KXMLGUIFactory>
#include <KStandardAction>
#include <KActionCollection>
#include <KServiceTypeTrader>

#include "BaseData.h"
#include "ModuleView.h"

SettingsBase::SettingsBase( QWidget * parent ) :
    KXmlGuiWindow(parent),
    categories( KServiceTypeTrader::self()->query("SystemSettingsCategory") ),
    modules( KServiceTypeTrader::self()->query("KCModule") )
{
    // Ensure delayed loading doesn't cause a crash
    activeView = 0;
    aboutDialog = 0;
    configDialog = 0;
    // Initialise search
    mainConfigGroup = KGlobal::config()->group( "Main" );
    searchText = new KLineEdit( this );
    searchText->setClearButtonShown( true );
    searchText->setClickMessage( i18nc( "Search through a list of control modules", "Search" ) );

    spacerWidget = new QWidget( this );
    spacerWidget->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Maximum );
    // Initalise the window so we don't flicker
    initToolBar();
    showTooltips = mainConfigGroup.readEntry( "ShowTooltips", true );
    // We can now launch the delayed loading safely
    QTimer::singleShot(0, this, SLOT(initApplication()));
}

SettingsBase::~SettingsBase()
{
    delete rootModule;
}

QSize SettingsBase::sizeHint() const
{
    return QSize(850, 650);
}

void SettingsBase::initApplication()
{
    // Prepare the menu of all modules
    rootModule = new MenuItem( true, 0 );
    initMenuList(rootModule);
    // Prepare the view area
    stackedWidget = new QStackedWidget( this );
    setCentralWidget(stackedWidget);
    // Prepare the Base Data
    BaseData::instance()->setMenuItem( rootModule );
    // Load all possible views
    KService::List pluginObjects = KServiceTypeTrader::self()->query( "SystemSettingsView" );
    for( int pluginsDone = 0; pluginsDone < pluginObjects.count(); pluginsDone = pluginsDone + 1 ) {
        KService::Ptr activeService = pluginObjects.at( pluginsDone );
        QString error;
        BaseMode * controller = activeService->createInstance<BaseMode>(this, QVariantList(), &error);
        if( error.isEmpty() ) {
            possibleViews.insert( activeService->library(), controller );
            controller->init( activeService );
            connect(controller, SIGNAL(changeToolBarItems(BaseMode::ToolBarItems)), this, SLOT(changeToolBar(BaseMode::ToolBarItems)));
            connect(controller, SIGNAL(actionsChanged()), this, SLOT(updateViewActions()));
            connect(searchText, SIGNAL(textChanged(const QString&)), controller, SLOT(searchChanged(const QString&)));
            connect(controller, SIGNAL(viewChanged()), this, SLOT(moduleChanged()));
            connect(controller->moduleView(), SIGNAL(configurationChanged(bool)), this, SLOT(toggleDirtyState(bool)));
        } else {
            kWarning() << "View load error: " + error;
        }
    }
    changePlugin();
}

void SettingsBase::initToolBar()
{
    // Fill the toolbar with default actions
    // Exit is the very last action
    actionCollection()->addAction( KStandardAction::Quit, "quit", this, SLOT( close() ) );
    // Configure goes at the end
    configureAction = actionCollection()->addAction( KStandardAction::Preferences, this, SLOT( configShow() ) );
    configureAction->setText( i18n("Configure") );
    // Help after it
    initHelpMenu();
    // Then a spacer so the search line-edit is kept separate
    spacerAction = new KAction( this );
    spacerAction->setDefaultWidget(spacerWidget);
    actionCollection()->addAction( "spacer", spacerAction );
    // Finally the search line-edit
    searchAction = new KAction( this );
    searchAction->setDefaultWidget(searchText);
    searchAction->setShortcut(KShortcut(QKeySequence(Qt::CTRL + Qt::Key_F)));
    connect( searchAction, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)),
         searchText, SLOT(setFocus()));
    actionCollection()->addAction( "searchText", searchAction );
    // Initialise the Window
    setupGUI(Save|Create,QString());
    menuBar()->hide();
    // Toolbar & Configuration
    setMinimumSize(800,480);
    toolBar()->setMovable(false); // We don't allow any changes
    changeToolBar( BaseMode::Search | BaseMode::Configure );
}

void SettingsBase::initHelpMenu()
{
    helpMenuObject = new KHelpMenu( this, KGlobal::activeComponent().aboutData() );
    helpActionMenu = new KActionMenu( KIcon("system-help"), i18n("Help"), this );
    helpActionMenu->setMenu( helpMenuObject->menu() );
    helpActionMenu->setDelayed( false );
    actionCollection()->addAction( "helpMenu", helpActionMenu );
    // Add the custom actions
    aboutModuleAction = actionCollection()->addAction( KStandardAction::AboutApp, "aboutModule", this, SLOT( about() ) );
    changeAboutMenu( 0, aboutModuleAction, i18n("About Active Module") );
    aboutViewAction = actionCollection()->addAction( KStandardAction::AboutApp, "aboutView", this, SLOT( about() ) );
    helpActionMenu->insertAction( helpMenuObject->action( KHelpMenu::menuAboutApp ), aboutModuleAction );
    helpActionMenu->insertAction( aboutModuleAction, aboutViewAction );
}

void SettingsBase::initConfig()
{
    // Prepare dialog first
    configDialog = new KDialog(this);
    configDialog->setButtons( KDialog::Ok | KDialog::Cancel );
    configWidget.setupUi(configDialog->mainWidget());
    configDialog->setCaption(i18n("Configure"));
    configDialog->setInitialSize(QSize(400,160));
    configDialog->restoreDialogSize( mainConfigGroup );
    // Get the list of modules
    foreach( BaseMode * mode, possibleViews ) {
        configWidget.CbPlugins->addItem( KIcon(mode->service()->icon()), mode->service()->name() );
    }
    connect(configDialog, SIGNAL(okClicked()), this, SLOT(configUpdated()));
}


void SettingsBase::initMenuList(MenuItem * parent)
{
    // look for any categories inside this level, and recurse into them
    for (int i = 0; i < categories.size(); ++i) {
        KService::Ptr entry = categories.at(i);
        QString parentCategory = entry->property("X-KDE-System-Settings-Parent-Category").toString();
        if ( parentCategory == parent->name() ) {
            MenuItem * menuItem = new MenuItem(true, parent);
            menuItem->setService( entry );
            initMenuList( menuItem );
        }
    }

    // scan for any modules at this level and add them
    for (int i = 0; i < modules.size(); ++i) {
        KService::Ptr entry = modules.at(i);
        QString category = entry->property("X-KDE-System-Settings-Parent-Category").toString();
        if(!parent->name().isEmpty() && category == parent->name() ) {
            // Add the module info to the menu
            MenuItem * infoItem = new MenuItem(false, parent);
            infoItem->setService( entry );
        }
    }
    parent->sortChildrenByWeight();
}

void SettingsBase::configUpdated()
{
    configDialog->saveDialogSize( mainConfigGroup );
    int currentIndex = configWidget.CbPlugins->currentIndex();
    mainConfigGroup.writeEntry( "ActiveView", possibleViews.keys().at(currentIndex) );
    showTooltips = configWidget.ChTooltips->isChecked();
    mainConfigGroup.writeEntry( "ShowTooltips", showTooltips );
    changePlugin();
}

void SettingsBase::configShow()
{
    // Initialise the configuration dialog if it hasn't already
    if( !configDialog ) {
        initConfig();
    }

    QStringList pluginList = possibleViews.keys();
    int configIndex = pluginList.indexOf(mainConfigGroup.readEntry( "ActiveView", "icon_mode" ));
    if( configIndex == -1 ) {
        configWidget.CbPlugins->setCurrentIndex( 0 );
    } else {
        configWidget.CbPlugins->setCurrentIndex( configIndex );
    }
    configWidget.ChTooltips->setChecked( showTooltips );
    if( pluginList.count() == 0 ) {
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
    mainConfigGroup.sync();
    return changes;
}

void SettingsBase::about()
{
    if( aboutDialog ) {
        delete aboutDialog;
        aboutDialog = 0;
    }

    const KAboutData * about = 0;
    if( sender() == aboutViewAction ) {
        about = activeView->aboutData();
    } else if( sender() == aboutModuleAction && activeView->moduleView() ) {
        about = activeView->moduleView()->aboutData();
    }

    if( about ) {
        aboutDialog = new KAboutApplicationDialog(about, 0);
        aboutDialog->show();
    }
}

void SettingsBase::changePlugin()
{
    if( possibleViews.count() == 0 ) { // We should ensure we have a plugin available to choose
        KMessageBox::error(this, i18n("System Settings was unable to find any views, and hence has nothing to display."), i18n("No views found"));
        return; // Halt now!
    }

    if( activeView ) {
        activeView->saveState();
    }

    QString viewToUse = mainConfigGroup.readEntry( "ActiveView", "icon_mode" );
    if( possibleViews.keys().contains(viewToUse) ) { // First the configuration entry
        activeView = possibleViews.value(viewToUse);
    }
    else { // Otherwise we activate the failsafe
        activeView = possibleViews.values().first();
    }

    if( stackedWidget->indexOf(activeView->mainWidget()) == -1 ) {
        stackedWidget->addWidget(activeView->mainWidget());
    }

    changeAboutMenu( activeView->aboutData(), aboutViewAction, i18n("About Active View") );
    moduleChanged();
    activeView->setEnhancedTooltipEnabled( showTooltips );
    stackedWidget->setCurrentWidget(activeView->mainWidget());
    updateViewActions();

    activeView->mainWidget()->setFocus();
}

void SettingsBase::toggleDirtyState(bool state)
{
    KCModuleInfo * moduleProxy = activeView->moduleView()->activeModule();
    configureAction->setDisabled(state);
    setCaption( moduleProxy->moduleName(), state );
}

void SettingsBase::updateViewActions()
{
    guiFactory()->unplugActionList( this, "viewActions" );
    guiFactory()->plugActionList( this, "viewActions", activeView->actionsList() );
}

void SettingsBase::moduleChanged()
{
    KCModuleInfo * moduleInfo = activeView->moduleView()->activeModule();
    if( moduleInfo ) {
        setCaption( moduleInfo->moduleName() );
    } else {
        setCaption( QString(), false );
    }
    changeAboutMenu( activeView->moduleView()->aboutData(), aboutModuleAction, i18n("About active module") );
}

void SettingsBase::changeToolBar( BaseMode::ToolBarItems toolbar )
{
    if( sender() != activeView ) {
        return;
    }
    guiFactory()->unplugActionList( this, "configure" );
    guiFactory()->unplugActionList( this, "search" );
    if ( BaseMode::Search & toolbar ) {
        QList<QAction*> searchBarActions;
        searchBarActions << spacerAction << searchAction;
        guiFactory()->plugActionList( this, "search", searchBarActions );
    }
    if ( BaseMode::Configure & toolbar ) {
        QList<QAction*> configureBarActions;
        configureBarActions << configureAction;
        guiFactory()->plugActionList( this, "configure", configureBarActions );
    }
}

void SettingsBase::changeAboutMenu( const KAboutData * menuAbout, KAction * menuItem, QString fallback )
{
    if( !menuItem ) {
        return;
    }

    if( menuAbout ) {
        menuItem->setText( i18n( "About %1", menuAbout->programName() ) );
        menuItem->setIcon( KIcon( menuAbout->programIconName() ) );
        menuItem->setEnabled(true);
    } else {
        menuItem->setText( fallback );
        menuItem->setIcon( KIcon( KGlobal::activeComponent().aboutData()->programIconName() ) );
        menuItem->setEnabled(false);
    }
}

#include "SettingsBase.moc"
