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
#include "BaseMode.h"
#include <iostream>

#include <QVariantList>

#include <KServiceTypeTrader>
#include <KAction>
#include <KStandardAction>
#include <ktoolbar.h>
#include <KConfig>
#include <KDebug>
#include <kactioncollection.h>
#include <KConfigGroup>
#include <KAboutData>
#include <KLineEdit>
#include <KMessageBox>

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
    // Load all possible views
    pluginObjects = KServiceTypeTrader::self()->query( "BaseMode" );
    for( int pluginsDone = 0; pluginsDone < pluginObjects.count(); pluginsDone = pluginsDone + 1 ) {
        KService::Ptr activeService = pluginObjects.at( pluginsDone );
        QString error;
        BaseMode * controller = activeService->createInstance<BaseMode>(this, QVariantList(), &error);
        if( error.isEmpty() ) {
            possibleViews.insert( activeService->library(), controller );
            controller->init( rootModule, activeService, KGlobal::config()->group( activeService->name() ) );
            connect(controller, SIGNAL(dirtyStateChanged(bool)), this, SLOT(toggleDirtyState(bool))); 
            connect(controller, SIGNAL(actionsChanged()), this, SLOT(updateViewActions()));
            connect(searchText, SIGNAL(textChanged(const QString&)), controller, SLOT(searchChanged(const QString&)));
            connect(controller, SIGNAL(moduleChange()), this, SLOT(moduleChanged()));
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
    toolBar()->addAction( configureAction );
    aboutAction = actionCollection()->addAction( KStandardAction::AboutApp, this, SLOT( about() ) );
    toolBar()->addAction( aboutAction );
    quitAction = actionCollection()->addAction( KStandardAction::Quit, this, SLOT( close() ) );
    toolBar()->addAction( quitAction );
    // We need to nominate the view to use
    initConfig();
    changePlugin();
}

SettingsBase::~SettingsBase()
{
}

void SettingsBase::initSearch()
{
    searchWidget = new QWidget( this );
    searchText = new KLineEdit( searchWidget );
    QLabel * searchIcon = new QLabel( searchWidget );
    searchIcon->setPixmap( BarIcon( "system-search" ) );
    QLabel * searchLabel = new QLabel( searchWidget );
    searchLabel->setText( i18n("Search modules: ") );
    QHBoxLayout * searchLayout = new QHBoxLayout( searchWidget );
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
    configDialog->setInitialSize(QSize(400,130));
    configDialog->restoreDialogSize( mainConfigGroup );
    // Get the list of modules
    foreach( BaseMode * mode, possibleViews.values() ) {
        configWidget.CbPlugins->addItem( KIcon(mode->service->icon()), mode->service->name() );
    }
    connect(configDialog, SIGNAL(okClicked()), this, SLOT(configUpdated()));
}

void SettingsBase::configUpdated()
{
    configDialog->saveDialogSize( mainConfigGroup );
    int currentIndex = configWidget.CbPlugins->currentIndex();
    mainConfigGroup.writeEntry( "ActiveView", possibleViews.keys().at(currentIndex) );
    changePlugin();
}

void SettingsBase::configShow()
{
    QStringList pluginList = possibleViews.keys();
    int configIndex = pluginList.indexOf(mainConfigGroup.readEntry( "ActiveView", "icon_mode" ));
    configWidget.CbPlugins->setCurrentIndex( configIndex );
    if( pluginList.count() == 0 ) {
        KMessageBox::error(this, i18n("KDE Control Center was unable to find any views, and subsequently nothing is available to configure"), i18n("No views found"));
    } else {
        configDialog->show();
    }
}

bool SettingsBase::queryClose()
{ 
    if( activeView ) {
        activeView->saveState();
    }
    mainConfigGroup.sync();
    return activeView->resolveDirtyState();
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
    if( activeView && activeView->activeModule() && activeView->activeModule()->aboutData() ) {
        listToAdd.append( activeView->activeModule()->aboutData() );
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
    setCentralWidget(activeView->mainWidget()); // Now we set it as the main widget
}

void SettingsBase::toggleDirtyState(bool state)
{ 
    KCModuleProxy * moduleProxy = activeView->activeModule(); 
    configureAction->setDisabled(state);
    setCaption( moduleProxy->moduleInfo().moduleName(), state );
}

void SettingsBase::initMenuList(MenuItem * parent)
{
    // look for any categories inside this level, and recurse into them
    int depth = 0;
    MenuItem * current = parent;
    while ( current && current->parent ) {
        depth++;
        current = current->parent;
    }

    QString space;
    space.fill( ' ', depth * 2 );
    kDebug() << space << "Looking for children in '" << parent->name << "'";
    for (int i = 0; i < categories.size(); ++i) {
        KService::Ptr entry = categories.at(i);
        QString parentCategory = entry->property("X-KDE-System-Settings-Parent-Category").toString();
        QString category = entry->property("X-KDE-System-Settings-Category").toString();
        if ( parentCategory == parent->name ) {
            KCModuleInfo module( entry->entryPath() );
            MenuItem * menuItem = new MenuItem(true, parent);
            menuItem->name = category;
            menuItem->service = entry;
            menuItem->item = module;
            initMenuList( menuItem );
        }
    }

    // scan for any modules at this level and add them
    for (int i = 0; i < modules.size(); ++i) {
        KService::Ptr entry = modules.at(i);
        QString category = entry->property("X-KDE-System-Settings-Parent-Category").toString();
        if(!parent->name.isEmpty() && category == parent->name ) {
            kDebug() << space << "found module '" << entry->name() << "' " << entry->entryPath();
            // Add the module info to the menu
            KCModuleInfo module(entry->entryPath());
            MenuItem * infoItem = new MenuItem(false, parent);
            infoItem->name = category;
            infoItem->service = entry;
            infoItem->item = module;
        }
    }
    parent->sortChildrenByWeight();
}

void SettingsBase::updateViewActions()
{
    foreach( KAction * oldAction, viewActions ) {
        toolBar()->removeAction( oldAction );
    }
    viewActions.clear();
    if( activeView ) {
        foreach( KAction * newAction, activeView->actionsList ) {
            toolBar()->addAction( newAction );
            viewActions.append( newAction );
        }
    }
}

void SettingsBase::moduleChanged()
{
    KCModuleProxy * moduleProxy = activeView->activeModule(); 
    if( moduleProxy ) {
        setCaption( moduleProxy->moduleInfo().moduleName() );
    } else {
        setCaption( "", false );
    }
}
