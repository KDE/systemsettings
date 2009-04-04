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

#include <QVariantList>

#include <KDebug>
#include <KConfig>
#include <KAboutData>
#include <KMessageBox>
#include <KCModuleInfo>
#include <KStandardAction>
#include <KActionCollection>
#include <KServiceTypeTrader>

#include "BaseData.h"
#include "ModuleView.h"

SettingsBase::SettingsBase( QWidget * parent ) :
    KXmlGuiWindow(parent),
    activeView( NULL ),
    categories( KServiceTypeTrader::self()->query("SystemSettingsCategory") ),
    modules( KServiceTypeTrader::self()->query("KCModule") )
{ 
    // Prepare the menu of all modules
    rootModule = new MenuItem( true, 0 );
    initMenuList(rootModule);
    initAbout();
    initSearch();
    // Prepare the view area
    stackedWidget = new QStackedWidget( this );
    setCentralWidget(stackedWidget);
    // Prepare the Base Data
    BaseData::instance()->setMenuItem( rootModule );
    // Load all possible views
    pluginObjects = KServiceTypeTrader::self()->query( "BaseMode" );
    for( int pluginsDone = 0; pluginsDone < pluginObjects.count(); pluginsDone = pluginsDone + 1 ) {
        KService::Ptr activeService = pluginObjects.at( pluginsDone );
        QString error;
        BaseMode * controller = activeService->createInstance<BaseMode>(this, QVariantList(), &error);
        if( error.isEmpty() ) {
            possibleViews.insert( activeService->library(), controller );
            controller->init( activeService );
            stackedWidget->addWidget(controller->mainWidget());
            connect(controller, SIGNAL(actionsChanged()), this, SLOT(updateViewActions()));
            connect(searchText, SIGNAL(textChanged(const QString&)), controller, SLOT(searchChanged(const QString&)));
            connect(controller, SIGNAL(viewChanged()), this, SLOT(moduleChanged()));
            connect(controller->moduleView(), SIGNAL(configurationChanged(bool)), this, SLOT(toggleDirtyState(bool))); 
        } else { 
            kWarning() << "View load error: " + error;
        }
    }
    // Toolbar & Configuration
    setMinimumSize(800,480);
    toolBar()->setMovable(false); // We don't allow any changes
    toolBar()->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mainConfigGroup = KGlobal::config()->group( "Main" );
    // Fill the toolbar with default actions
    searchAction = new KAction( this );
    searchAction->setDefaultWidget(searchWidget);
    actionCollection()->addAction( "searchText", searchAction );
    toolBar()->addAction( searchAction );
    configureAction = actionCollection()->addAction( KStandardAction::Preferences, this, SLOT( configShow() ) );
    configureAction->setText( i18n("Configure") );
    toolBar()->addAction( configureAction );
    aboutAction = actionCollection()->addAction( KStandardAction::AboutApp, this, SLOT( about() ) );
    aboutAction->setText( i18n("About") );
    toolBar()->addAction( aboutAction );
    // We need to nominate the view to use
    showTooltips = mainConfigGroup.readEntry( "ShowTooltips", false );
    initConfig();
    changePlugin();
}

SettingsBase::~SettingsBase()
{
    delete rootModule;
}

void SettingsBase::initSearch()
{
    searchWidget = new QWidget( this );
    searchText = new KLineEdit( searchWidget );
    searchText->setClearButtonShown( true );
    QLabel * searchIcon = new QLabel( searchWidget );
    searchIcon->setBuddy( searchText );
    searchIcon->setPixmap( BarIcon( "system-search" ) );
    QLabel * searchLabel = new QLabel( searchWidget );
    searchLabel->setBuddy( searchText );
    searchLabel->setText( i18nc("Search through a list of control modules","Search:") );
    QHBoxLayout * searchLayout = new QHBoxLayout( searchWidget );
    searchLayout->setMargin( 0 );
    searchLayout->setSpacing( KDialog::spacingHint() );
    searchLayout->addWidget( searchIcon );
    searchLayout->addWidget( searchLabel );
    searchLayout->addWidget( searchText );
    searchWidget->setLayout( searchLayout );
}

void SettingsBase::initConfig()
{   // Prepare dialog first
    configDialog = new KDialog(this);
    configDialog->setButtons( KDialog::Ok | KDialog::Cancel );
    configWidget.setupUi(configDialog->mainWidget());
    configDialog->setCaption(i18n("Configure"));
    configDialog->setInitialSize(QSize(400,140));
    configDialog->restoreDialogSize( mainConfigGroup );
    // Get the list of modules
    foreach( BaseMode * mode, possibleViews.values() ) {
        configWidget.CbPlugins->addItem( KIcon(mode->service()->icon()), mode->service()->name() );
    }
    connect(configDialog, SIGNAL(okClicked()), this, SLOT(configUpdated()));
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
    QStringList pluginList = possibleViews.keys();
    int configIndex = pluginList.indexOf(mainConfigGroup.readEntry( "ActiveView", "icon_mode" ));
    configWidget.CbPlugins->setCurrentIndex( configIndex );
    configWidget.ChTooltips->setChecked( showTooltips );
    if( pluginList.count() == 0 ) {
        KMessageBox::error(this, i18n("KDE Control Center was unable to find any views, and subsequently nothing is available to configure"), i18n("No views found"));
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

void SettingsBase::initAbout()
{
    aboutDialog = new KPageDialog(this); // We create it on the first run
    aboutDialog->setPlainCaption( i18n("About KDE Control Center") );
    aboutDialog->setButtons( KDialog::Close );
}

void SettingsBase::about()
{
    QList<const KAboutData *> listToAdd;

    // First we cleanup from previous runs
     while (!aboutAppPage.isEmpty()) {
         aboutDialog->removePage(aboutAppPage.takeFirst());
     }
    // Build the list of About Items to add
    if( KGlobal::activeComponent().aboutData() ) {
        listToAdd.append( KGlobal::activeComponent().aboutData() );
    }
    if( activeView && activeView->aboutData() ) {
        listToAdd.append( activeView->aboutData() );
    }
    if( activeView && activeView->moduleView() && activeView->moduleView()->aboutData() ) {
        listToAdd.append( activeView->moduleView()->aboutData() );
    }
    foreach( const KAboutData * addingItem, listToAdd ) {
        KAboutApplicationDialog * addingDialog = new KAboutApplicationDialog(addingItem, 0);
        KPageWidgetItem * addingPage = new KPageWidgetItem( addingDialog, addingItem->programName() );
        addingDialog->setButtons( KDialog::None );
        addingPage->setHeader( "" );
        addingPage->setIcon( KIcon(addingItem->programIconName()) );
        aboutDialog->addPage(addingPage);
        aboutAppPage.append(addingPage);
    }
    aboutDialog->show();
}

void SettingsBase::changePlugin()
{
    if( possibleViews.count() == 0 ) // We should ensure we have a plugin available to choose 
    {   KMessageBox::error(this, i18n("KDE Control Center was unable to find any views, and subsequently cannot display anything"), i18n("No views found"));
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

    activeView->setEnhancedTooltipEnabled( showTooltips );
    stackedWidget->setCurrentWidget(activeView->mainWidget());
    updateViewActions();

    searchText->setFocus();
}

void SettingsBase::toggleDirtyState(bool state)
{ 
    KCModuleInfo * moduleProxy = activeView->moduleView()->activeModule(); 
    configureAction->setDisabled(state);
    setCaption( moduleProxy->moduleName(), state );
}

void SettingsBase::initMenuList(MenuItem * parent)
{
    // look for any categories inside this level, and recurse into them
    int depth = 0;
    MenuItem * current = parent;
    while ( current && current->parent() ) {
        depth++;
        current = current->parent();
    }

    QString space;
    space.fill( ' ', depth * 2 );
    kDebug() << space << "Looking for children in '" << parent->name() << "'";
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
            kDebug() << space << "found module '" << entry->name() << "' " << entry->entryPath();
            // Add the module info to the menu
            MenuItem * infoItem = new MenuItem(false, parent);
            infoItem->setService( entry );
        }
    }
    parent->sortChildrenByWeight();
}

void SettingsBase::updateViewActions()
{
    foreach( QAction * oldAction, viewActions ) {
        toolBar()->removeAction( oldAction );
    }
    viewActions.clear();
    if( activeView ) {
        QAction *before = toolBar()->actions().value( 0 );
        toolBar()->insertActions( before, activeView->actionsList() );
        viewActions << activeView->actionsList() << toolBar()->insertSeparator( before );
    }
}

void SettingsBase::moduleChanged()
{
    KCModuleInfo * moduleInfo = activeView->moduleView()->activeModule();
    if( moduleInfo ) {
        setCaption( moduleInfo->moduleName() );
    } else {
        setCaption( QString(), false );
    }
}

#include "SettingsBase.moc"
