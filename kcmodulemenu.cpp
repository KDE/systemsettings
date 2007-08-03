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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kcmodulemenu.h"

#include <kservicegroup.h>
#include <kdebug.h>
#include <kservicetypetrader.h>
#include <kauthorized.h>
#include <QList>

class KCModuleMenuPrivate {
public:
	KCModuleMenuPrivate(){
	}
				
	QMap<QString, QList<MenuItem> > menus;
	QString basePath;
};

KCModuleMenu::KCModuleMenu( const QString &menuName ) :
	d( new KCModuleMenuPrivate )
{
	// Make sure we can find the menu
	QString menuRoot = "System Settings"; //just a handy key to use, not part of UI
	d->basePath = menuRoot + '/';
	readMenu( "", menuRoot );

	QMapIterator<QString, QList<MenuItem> > i(d->menus);
	/*debugging
	while (i.hasNext()) {
		i.next();
		kDebug() << "menu: " << i.key();
		QList<MenuItem> items = i.value();
		for (int i = 0; i < items.size(); ++i) {
			kDebug() << "  item menu: " << items.at(i).caption <<  endl;
			
		}
	}
	*/
	KService::List offers = KServiceTypeTrader::self()->query("KCModule");
}

KCModuleMenu::~KCModuleMenu()
{
	delete d;
}

void KCModuleMenu::readMenu( const QString &parentName, const QString &caption )
{
	typedef KSharedPtr<KService> MySharedPtr_KService;

	QList<MenuItem> currentMenu;
	KService::List categories = KServiceTypeTrader::self()->query("SystemSettingsCategory");
	for (int i = 0; i < categories.size(); ++i) {
		const KService* entry = categories.at(i).data();
		QString parentCategory = entry->property("X-KDE-System-Settings-Parent-Category").toString();
		QString category = entry->property("X-KDE-System-Settings-Category").toString();

		if ( parentCategory == parentName ){
			MenuItem menuItem(true);
			menuItem.caption = entry->name();
			menuItem.subMenu = caption + '/' + entry->name() + '/';
			currentMenu.append( menuItem );

			readMenu( category, caption + '/' + entry->name() );
		}
	}

	KService::List modules = KServiceTypeTrader::self()->query("KCModule");
	for (int i = 0; i < modules.size(); ++i) {
		const KService* entry = modules.at(i).data();
		QString category = entry->property("X-KDE-System-Settings-Parent-Category").toString();
		if( category == parentName && !parentName.isEmpty() ) {
			// Add the module info to the menu
			KCModuleInfo module(entry->desktopEntryPath());
			append(module);
			MenuItem infoItem(false);
			infoItem.caption = entry->name();
			infoItem.item = module;
			currentMenu.append( infoItem );
		}
	}

	d->menus.insert( caption + '/', currentMenu );
}

bool KCModuleMenu::addEntry( KSycocaEntry *entry ){
	if( !entry->isType(KST_KService) )
		return false;
	
  KSharedPtr<KService> service(static_cast<KService*>(entry));

	if ( !KAuthorized::authorizeControlModule( service->menuId()) ) {
		return false;
  }

	KCModuleInfo module( service );
	if ( module.library().isEmpty() )
		return false;

	return true;
}


QList<KCModuleInfo> KCModuleMenu::modules( const QString &menuPath )
{
	QList<KCModuleInfo> list;

	QList<MenuItem> subMenu = menuList(menuPath);
	QList<MenuItem>::iterator it;
	for ( it = subMenu.begin(); it != subMenu.end(); ++it ){
		if ( !(*it).menu )
			list.append( (*it).item );
	}

	return list;
}

QStringList KCModuleMenu::submenus( const QString &menuPath )
{
	QStringList list;

	QList<MenuItem> subMenu = menuList(menuPath);
	QList<MenuItem>::iterator it;
	for ( it = subMenu.begin(); it != subMenu.end(); ++it ){
		if ( (*it).menu )
			list.append( (*it).subMenu );
	}

	return list;
}

QList<MenuItem> KCModuleMenu::menuList( const QString &menuPath )
{
	if( menuPath.isEmpty() ) {
		if( d->basePath.isEmpty())
			return QList<MenuItem>();
		else
			return menuList( d->basePath );
	}
	return d->menus[menuPath];
}
