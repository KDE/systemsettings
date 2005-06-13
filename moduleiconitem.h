/**
 * This file is part of the System Settings package
 * Copyright (C) 2005 Benjamin C Meyer
 *                    <ben+systempreferences at meyerhome dot net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef MODULEICONITEM_H
#define MODULEICONITEM_H

#include <kiconview.h>
#include <kcmoduleinfo.h>

class ConfigModule;

/**
 * Stores information about what modules goes with this item.
 * Also provides means of loading the enabled/disabled image (see kcmsearch).
 */
class ModuleIconItem : public QIconViewItem
{

public:
  ModuleIconItem( KIconView *parent, KCModuleInfo module );
	
  ModuleIconItem( KIconView *parent, const QString &text,
									const QString &imageName );

	/**
	 * Update the icon to either be enabled or not.
	 */
	void loadIcon( bool enabled = true );

	// The modules that go with this item
	QValueList<KCModuleInfo> modules;
					
private:
	int currentState;
	QString imageName;
};

#endif // MODULEICONITEM_H

