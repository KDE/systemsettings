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
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kcmodulemenu.h"

#include <kapplication.h>
#include <kservicegroup.h>
#include <kdebug.h>
#include <qdict.h>

class KCModuleMenuPrivate {
public:
	KCModuleMenuPrivate(){
	}
				
	QMap<QString, QValueList<MenuItem> > menus;
	QString basePath;
};

KCModuleMenu::KCModuleMenu( const QString &menuName ) :
	d( new KCModuleMenuPrivate )
{
	kdDebug() << "MenuName: \"" << menuName << "\"." << endl;
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

	caption = group->caption();
	QValueList<MenuItem> currentMenu;
			
	for( KServiceGroup::List::ConstIterator it = list.begin();
			 it != list.end(); it++)
	{
		KSycocaEntry *entry = (*it);
		if( addEntry(entry) ) {
			KCModuleInfo module((KService*)entry);
			append(module);
			MenuItem infoItem(false);
			infoItem.caption = this->deriveCaptionFromPath(entry->name());
			infoItem.item = module;
			currentMenu.append( infoItem );
		}

		if ( entry->isType(KST_KServiceGroup) ){
			MenuItem menuItem(true);
			menuItem.caption = this->deriveCaptionFromPath(entry->name());
			menuItem.subMenu = entry->entryPath();
			currentMenu.append( menuItem );

			readMenu( entry->entryPath() );
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
	QValueList<KCModuleInfo> list;

	QValueList<MenuItem> subMenu = menuList(menuPath);
	QValueList<MenuItem>::iterator it;
	for ( it = subMenu.begin(); it != subMenu.end(); ++it ){
		if ( !(*it).menu )
			list.append( (*it).item );
	}

	return list;
}

QStringList KCModuleMenu::submenus( const QString &menuPath )
{
	QStringList list;

	QValueList<MenuItem> subMenu = menuList(menuPath);
	QValueList<MenuItem>::iterator it;
	for ( it = subMenu.begin(); it != subMenu.end(); ++it ){
		if ( (*it).menu )
			list.append( (*it).subMenu );
	}

	return list;
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

/*
 * Okay, I think there could be a much more elegant way of doing
 * this... but I'm having a hell fo a time figuring it out.
 *
 * The purpose of this function is to take a menu path and turn it
 * into a caption that we can put in a tab.  Why do it this way?  I
 * don't know, you tell me.  Before I started hacking this we used a
 * radio control with two buttons (or so it seemed, I could be wrong)
 * with General and Advanced in a ui.rc file.
 *
 * Now that we're using tabs, we no longer have that UI file giving us
 * the names for the tabs, and since I didn't want to hard-code
 * anything, and since KSycocaEntry stuff doesn't give you a nice way
 * (that I noticed anyway) to figure out what your caption should be,
 * I decided that cleverness is lost on this problem.  So screw it,
 * I'll just parse the stupid path and be done with it.
 *
 * This function is certainly nothing short of dull and boring and
 * routine, but I figured that this might require a bit of explanation
 * since it just seems kinda silly to do it this way to me.  I guess I
 * never know... I could be doing it the best way.
 *
 * "Michael D. Stemle, Jr." <manchicken@notsosoft.net>
 */
QString KCModuleMenu::deriveCaptionFromPath( const QString &menuPath )
{
	QStringList parts(QStringList::split("/",menuPath));
	QString result("");

	QStringList::Iterator it = parts.end(); // Start at the end

	// Find the last non-empty string in the split.
	for (; it != parts.begin(); --it) {
		if (!((*it).isNull()) && !((*it).isEmpty())) {
			result += *it;
			return result;
		}
	}
}
