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

SettingsBase::SettingsBase( QWidget * parent ) :
    KXmlGuiWindow(parent),
    categories( KServiceTypeTrader::self()->query("SystemSettingsCategory") ),
    modules( KServiceTypeTrader::self()->query("KCModule") )
{ 
  // Prepare the menu of all modules
  rootModule = new MenuItem( true, 0 );
  initMenuList(rootModule);
  // Load all possible views
  pluginObjects = KServiceTypeTrader::self()->query( "BaseMode" );
  for( int pluginsDone = 0; pluginsDone < pluginObjects.count(); pluginsDone = pluginsDone + 1 ) {
      const KService * activeService = pluginObjects.at( pluginsDone ).data();
      QString error;
      BaseMode * controller = activeService->createInstance<BaseMode>(this, QVariantList(), &error);
      if( controller ) {
          possibleViews.insert( activeService->library(), controller );
          controller->rootItem = rootModule;
          connect(controller, SIGNAL(dirtyStateChanged(bool)), this, SLOT(toggleConfiguration(bool))); 
      } else { 
          kWarning() << error;
      }
  }
  // Toolbar & Configuration
  toolBar()->setMovable(false); // We don't allow any changes
  mainConfigGroup = KGlobal::config()->group( "Main" );
  // Fill the toolbar with default actions
  configureAction = actionCollection()->addAction( KStandardAction::Preferences, this, SLOT( configNow() ) );
  toolBar()->addAction( configureAction );
  aboutAction = actionCollection()->addAction( KStandardAction::AboutApp, this, SLOT( about() ) );
  toolBar()->addAction( aboutAction );
  quitAction = actionCollection()->addAction( KStandardAction::Quit, this, SLOT( goingToQuit() ) );
  toolBar()->addAction( quitAction );
  // We need to nominate the view to use
  changePlugin();
}

SettingsBase::~SettingsBase()
{
}

void SettingsBase::configNow()
{
}

void SettingsBase::configUpdated()
{
}

void SettingsBase::goingToQuit()
{
}

void SettingsBase::about()
{
}

void SettingsBase::changePlugin()
{
  if( possibleViews.count() == 0 ) // We should ensure we have a plugin available to choose 
  { return; } // Halt now!

  QString viewToUse = mainConfigGroup.readEntry( "ActiveView", "MacView" );
  if( possibleViews.keys().contains(viewToUse) ) { // First the configuration entry
      activeView = possibleViews.value(viewToUse);
  }
  else { // Otherwise we activate the failsafe
      activeView = possibleViews.values().first();
  }
  setCentralWidget(activeView->mainWidget()); // Now we set it as the main widget
}

void SettingsBase::toggleConfiguration(bool state)
{ configDirty = state;
  configureAction->setDisabled(state);
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
            kDebug() << space << "filename is " << module.fileName();
            MenuItem * infoItem = new MenuItem(false, parent);
            infoItem->name = category;
            infoItem->service = entry;
            infoItem->item = module;
        }
    }

    parent->sortChildrenByWeight();
}