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
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "moduleiconitem.h"
#include <kiconloader.h>

#define IMAGE_SIZE 32

ModuleIconItem::ModuleIconItem( KIconView *parent, KCModuleInfo module)
	: QIconViewItem( parent, module.moduleName(),
		SmallIcon( module.icon(), IMAGE_SIZE ) ),
		currentState( KIcon::ActiveState), imageName(module.icon())
{
	modules.append(module);
}

ModuleIconItem::ModuleIconItem( KIconView *parent, const QString &text,
		const QString &imageName )
	: QIconViewItem( parent, text, SmallIcon( imageName, IMAGE_SIZE ) ),
			currentState( KIcon::ActiveState )
{
	this->imageName = imageName;
}

void ModuleIconItem::loadIcon( bool enabled )
{
	int newState = enabled ? KIcon::DefaultState : KIcon::DisabledState;
	if( newState == currentState )
		return;

	currentState = newState;
	setPixmap( SmallIcon( imageName, IMAGE_SIZE, currentState ) );
}

