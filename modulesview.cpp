/**
 * This file is part of the System Preferences package
 * Copyright (C) 2005 Benjamin C Meyer (ben+systempreferences at meyerhome dot net)
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

#include "modulesview.h"

#include <kiconview.h>
#include <qvbox.h>
#include <qlabel.h>
#include <klocale.h>
#include <kservicegroup.h>
#include <qlayout.h>
#include <kiconloader.h>
#include <kcmoduleloader.h>
#include <kcmultidialog.h>
#include <kdialogbase.h>
#include <kiconviewsearchline.h>
#include <kapplication.h>
#include <kaboutapplication.h>
#include <kdebug.h>
#include <qhbox.h>
#include "kcmsearch.h"

#include "moduleiconitem.h"
#include "kcmodulemenu.h"

ModulesView::ModulesView( KCModuleMenu *menu, QWidget *parent, const char *name ) : QWidget( parent, name ), myMenu( menu )
{
	QVBoxLayout *layout = new QVBoxLayout( this, 11, 6, "layout" );
	layout->setAutoAdd( true );
}

ModulesView::~ModulesView()
{
}

void ModulesView::createRow( const QString &parentPath )
{
	KServiceGroup::Ptr group = KServiceGroup::group(parentPath);
	QString defName = parentPath.left( parentPath.length()-1 );
	int pos = defName.findRev( '/' );
	if ( pos >= 0 )
		defName = defName.mid( pos+1 );

	if ( !group || !group->isValid() ){
		kdDebug() << "Invalid Group \"" << parentPath << "\"."<< endl;
		return;
	}

	// Make IconView
	RowIconView *iconView = new RowIconView( this );
	iconView->setFrameShape( RowIconView::StyledPanel );
	iconView->setLineWidth( 0 );
	iconView->setSpacing( 16 );
	iconView->setItemsMovable(false);
	groups.append(iconView);
	connect(iconView, SIGNAL( clicked( QIconViewItem* ) ),
		      this, SIGNAL( itemSelected( QIconViewItem* ) ) );

	// Load submenus first
	QStringList subMenus = myMenu->submenus( parentPath );
	for(QStringList::ConstIterator it = subMenus.begin(); it != subMenus.end(); ++it)
	{
		QString path = *it;

		KServiceGroup::Ptr group = KServiceGroup::group( path );
		QString defName = path.left( path.length()-1 );
		int pos = defName.findRev( '/' );
		if ( pos >= 0 )
			 defName = defName.mid( pos + 1 );
		if ( group && group->isValid() && group.count() > 0 ) {
			ModuleIconItem *item = new ModuleIconItem( ((KIconView*)iconView), defName, group->icon() );
			item->modules = myMenu->modules( path );
		}
	}

	// Then load the individual items
	QValueList<KCModuleInfo> list = myMenu->modules( parentPath );
	for( QValueList<KCModuleInfo>::iterator it = list.begin(); it != list.end(); ++it )
	{
		new ModuleIconItem( iconView, *it );
	}

	// Force the height for those items that have two words.
	iconView->setMinimumHeight( iconView->minimumSizeHint().height() );
	setPaletteBackgroundColor( groups[0]->paletteBackgroundColor() );
}

void ModulesView::createRow( const QStringList &items )
{
	// Make IconView
	RowIconView *iconView = new RowIconView( this );
	iconView->setFrameShape( RowIconView::StyledPanel );
	iconView->setLineWidth( 0 );
	iconView->setSpacing( 16 );
	iconView->setItemsMovable(false);
	groups.append(iconView);
	connect(iconView, SIGNAL( clicked( QIconViewItem* ) ),
		      this, SIGNAL( itemSelected( QIconViewItem* ) ) );

	for(QStringList::ConstIterator it = items.begin(); it != items.end(); ++it)
	{
		ModuleIconItem *item = new ModuleIconItem( iconView, (*it).section('/',1,1), "folder" );
		item->modules = myMenu->modules( *it );
	}

	// Force the height for those items that have two words.
	iconView->setMinimumHeight( iconView->minimumSizeHint().height() );
	setPaletteBackgroundColor( groups[0]->paletteBackgroundColor() );
}

