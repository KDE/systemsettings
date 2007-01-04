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

#ifndef KCMMODULEMENU_H
#define KCMMODULEMENU_H

#include <kcmoduleinfo.h>

class KCModuleMenuPrivate;

/**
 * List of all KCM modules inside a FreeDesktop.org menu
 * The menu spec is located at: http://www.freedesktop.org/Standards/menu-spec
 *
 * For a menu to show up in KDE three files need to be installed in the system.
 * 
 * example-merge.menu
 * example.directory
 * example.menu
 *
 * example-merge.menu should be installed in xdg/menus/applications-merged/
 * so that ksyscoco will find it.
 *
 * \code
 * <!DOCTYPE Menu PUBLIC "-//freedesktop//DTD Menu 1.0//EN"
 *   "http://www.freedesktop.org/standards/menu-spec/1.0/menu.dtd">
 *
 * <Menu>
 *   <!-- The following menus are hidden by default -->
 *   <Menu>
 *     <Name>Example Menu</Name>
 *     <Directory>example.directory</Directory>
 *     <MergeFile>../example.menu</MergeFile>
 *   </Menu>
 * </Menu>
 * \endcode
 *
 * example.directory should be installed in share/desktop-directories/ where files
 * such as kde-system.directory reside.  It is important that it have X-KDE-BaseGroup
 * as this value is the class constructor argument.
 * 
 * \code
 * [Desktop Entry]
 * Encoding=UTF-8
 * Name=Example Menu
 * NoDisplay=true
 * Icon=package_settings
 * X-KDE-BaseGroup=examplemenu
 * \endcode
 * 
 * example.menu should be installed in xdg/menus/ so that ksyscoco will find
 * it. See the above url for example menus.  After changing the menu you need
 * to run "kbuildsycoca" to regenerate the cache as ksyscoco will cache the
 * menu and is a file that doesn't change on users.
 */

/**
 * A menu consists of menu items.  An item is either another menu or a module.
 */
class MenuItem {
public:
	MenuItem( bool isMenu=false ){ menu = isMenu; };	
	bool menu;
	QString subMenu;
	QString caption;
	KCModuleInfo item;
};


class KCModuleMenu : public QValueList<KCModuleInfo>
{

public:
	QString caption;

	/**
	 * @param the X-KDE-BaseGroup item from the directory file
	 * that should be loaded.
	 *
	 * Example:
	 * In example.directory
	 * X-KDE-BaseGroup=examplemenu
	 * so menuName should be "systemsettings"
	 */
	KCModuleMenu( const QString &menuName );

	/**
	 * Deconstructor
	 */
	virtual ~KCModuleMenu();

	/**
	 * Returns item of a menu path. An empty string is the top level.
	 * Item order is maintained from the menu file.
	 * @param path to return submenus from.
	 * @return all items in menuPath.
	 */
	QValueList<MenuItem> menuList( const QString &menuPath=QString::null );

	/**
	 * Returns the modules in a menu path. An empty string is the top level.
	 * @param menu to return modules from.
	 * @returns only the top level modules of menuPath
	 */
	QValueList<KCModuleInfo> modules( const QString &menuPath=QString::null );

	/**
	 * Returns the sub menus of a menu path. An empty string is the top level.
	 * @param path to return submenus from.
	 * @return only the submenus of menuPath.
	 */
	QStringList submenus( const QString &menuPath=QString::null );


protected:
	/**
	 * Reads all the desktop files from the pathName and saves/loads
	 * all of the kcm's into the list.
	 * This is the <Name> from the merge.menu file
	 * @param pathName the base path name of the menu.
	 */
	virtual void readMenu( const QString &pathName );

	/**
	 * Function that determines if the entry from readMenu
	 * should be added to the list or not.  It checks
	 * to see if it is an authorized kcm module that has a library.
	 *
	 * Overload to filter out kcm modules
	 * 
	 * @entry to test
	 * @return true if it should be included
	 */
	virtual bool addEntry( KSycocaEntry *entry );

private:
	KCModuleMenuPrivate *d;
	QString deriveCaptionFromPath( const QString &menuPath );
};

#endif // KCMMODULEMENU_H

