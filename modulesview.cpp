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

#include "kcmsearch.h"

#include "moduleiconitem.h"
#include "kcmodulemenu.h"

ModulesView::ModulesView( const QString &menuName, QWidget *parent,
		const char *name ) : QWidget( parent, name ), menu( NULL )
{
	menu = new KCModuleMenu( menuName );
	QVBoxLayout *layout = new QVBoxLayout( this, 11, 6, "layout" );
	
	QStringList subMenus = menu->submenus();
	// Go through and load the top two levels
	QStringList::ConstIterator it;
	for( it = subMenus.begin(); it != subMenus.end(); ++it ){
		// After the first time around add a line
		if(it != subMenus.begin()){
			QFrame *line = new QFrame( this, "line");
			line->setFrameShadow( QFrame::Sunken );
			line->setFrameShape( QFrame::HLine );
			layout->addWidget( line );
		}

		// Build the row if modueles/icons
		createRow( QString(*it), layout );
	}

	// Make empty iconView for the search widget
	if( groups.count()==0 ) {
		RowIconView *iconView = new RowIconView( this, "groupiconview" );
		iconView->setPaletteBackgroundColor( paletteBackgroundColor() );
		iconView->setLineWidth( 0 );
		groups.append(iconView);
	}
}

ModulesView::~ModulesView()
{
	if(menu)
		delete menu;
}

void ModulesView::createRow( const QString &parentPath, QBoxLayout *boxLayout )
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

	// Make header
	QHBoxLayout *rowLayout = new QHBoxLayout( 0, 0, 6, "rowLayout" );

	// Heaer Icon
	QLabel *icon = new QLabel( this, "groupicon" );
	icon->setPixmap( SmallIcon( group->icon() ) );
	icon->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1,
		(QSizePolicy::SizeType)5, 0, 0, icon->sizePolicy().hasHeightForWidth() ) );
	rowLayout->addWidget( icon );

	// Header Name
	QLabel *textLabel = new QLabel( this, "groupcaption" );
	textLabel->setText( group->caption() );
	textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7,
		(QSizePolicy::SizeType)5, 0, 0, textLabel->sizePolicy().hasHeightForWidth()));
	rowLayout->addWidget( textLabel );

	boxLayout->addLayout( rowLayout );

	// Make IconView
	RowIconView *iconView = new RowIconView( this, "groupiconview" );
	iconView->setPaletteBackgroundColor( paletteBackgroundColor() );
	iconView->setFrameShape( RowIconView::StyledPanel );
	iconView->setLineWidth( 0 );
	iconView->setSpacing( 6 );
	iconView->setItemsMovable(false);
	groups.append(iconView);
	connect(iconView, SIGNAL( clicked( QIconViewItem* ) ),
		      this, SIGNAL( itemSelected( QIconViewItem* ) ) );
	boxLayout->addWidget( iconView );

	// Load submenus first
	QStringList subMenus = menu->submenus( parentPath );
	for(QStringList::ConstIterator it = subMenus.begin();
		it != subMenus.end(); ++it)
	{
		QString path = *it;

		KServiceGroup::Ptr group = KServiceGroup::group( path );
		QString defName = path.left( path.length()-1 );
		int pos = defName.findRev( '/' );
		if ( pos >= 0 )
			 defName = defName.mid( pos + 1 );
		if ( group && group->isValid() && group.count() > 0 ) {
			ModuleIconItem *item = new ModuleIconItem( ((KIconView*)iconView), defName, group->icon() );
			item->modules = menu->modules( path );
		}
	}

	// Then load the individual items
  QValueList<KCModuleInfo>::iterator it;
  QValueList<KCModuleInfo> list = menu->modules( parentPath );
	for ( it = list.begin(); it != list.end(); ++it ){
		ModuleIconItem *item = new ModuleIconItem( iconView, *it );
	}
	
	// Force the height for those items that have two words.
	iconView->setMinimumHeight( iconView->minimumSizeHint().height() );
}

