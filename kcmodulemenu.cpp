/**
 * Copyright (C) 2005 Benjamin C Meyer (ben+kcmodulemenu at meyerhome dot net)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "kcmodulemenu.h"

#include <kapplication.h>
#include <kservicegroup.h>
#include <kdebug.h>
#include <qdict.h>

class KCModuleMenuPrivate {
public:
	KCModuleMenuPrivate(){
		subMenus.setAutoDelete(true);
	}
				
	class ModuleMenu {
	public:
		QValueList<KCModuleInfo> modules;
		QStringList submenus;
	};
	
	QMap<QString, QValueList<MenuItem> > menus;
	QDict<ModuleMenu> subMenus;
	QString basePath;
};

KCModuleMenu::KCModuleMenu( const QString &menuName ) :
	d( new KCModuleMenuPrivate )
{
	// Make sure we can find the menu
	KServiceGroup::Ptr serviceGroup = KServiceGroup::baseGroup( menuName );
	if( !serviceGroup ){
		kdDebug() << "Unable to load menu \"" << menuName << 
						"\" from KServiceGroup." << endl;
		return;
	}

	d->basePath = serviceGroup->relPath();
	readMenu( d->basePath );
}

KCModuleMenu::~KCModuleMenu()
{
	delete d;
}

void KCModuleMenu::readMenu( const QString &pathName )
{
	KServiceGroup::Ptr group = KServiceGroup::group( pathName );
	if ( !group || !group->isValid() )
		return;

	KServiceGroup::List list = group->entries( true, true );
	if( list.isEmpty() )
		return;

	KCModuleMenuPrivate::ModuleMenu *menu = new KCModuleMenuPrivate::ModuleMenu;
	d->subMenus.insert( pathName, menu );

	QValueList<MenuItem> currentMenu;
			
	for( KServiceGroup::List::ConstIterator it = list.begin();
			 it != list.end(); it++)
	{
		KSycocaEntry *entry = (*it);
		if( addEntry(entry) ) {
			KCModuleInfo module((KService*)entry);
			append(module);
			menu->modules.append(module);

			MenuItem infoItem(false);
			infoItem.item = module;
			currentMenu.append( infoItem );
		}

		if ( entry->isType(KST_KServiceGroup) ){
			MenuItem menuItem(true);
			menuItem.subMenu = entry->entryPath();
			currentMenu.append( menuItem );

			readMenu( entry->entryPath() );
			menu->submenus.append( entry->entryPath() );
		}
	}

	d->menus.insert( pathName, currentMenu );
}

bool KCModuleMenu::addEntry( KSycocaEntry *entry ){
	if( !entry->isType(KST_KService) )
		return false;
	
	KService *service = static_cast<KService*>( entry );
	if ( !kapp->authorizeControlModule( service->menuId()) )
		return false;

	KCModuleInfo module( service );
	if ( module.library().isEmpty() )
		return false;

	return true;
}


QValueList<KCModuleInfo> KCModuleMenu::modules( const QString &menuPath )
{
	if( menuPath.isEmpty() ) {
		if( d->basePath.isEmpty())
			return QValueList<KCModuleInfo>();
		else
			return modules( d->basePath );
	}
	
	KCModuleMenuPrivate::ModuleMenu *subMenu = d->subMenus.find( menuPath );
	if( subMenu )
		return subMenu->modules;

	return QValueList<KCModuleInfo>();
}

QStringList KCModuleMenu::submenus( const QString &menuPath )
{
	if( menuPath.isEmpty() ) {
		if( d->basePath.isEmpty())
			return QStringList();
		else
			return submenus( d->basePath );
	}

	KCModuleMenuPrivate::ModuleMenu *subMenu = d->subMenus.find( menuPath );
	if( subMenu )
		return subMenu->submenus;

	return QStringList();
}

QValueList<MenuItem> KCModuleMenu::menuList( const QString &menuPath )
{
	if( menuPath.isEmpty() ) {
		if( d->basePath.isEmpty())
			return QValueList<MenuItem>();
		else
			return menuList( d->basePath );
	}
	return d->menus[menuPath];	
}

