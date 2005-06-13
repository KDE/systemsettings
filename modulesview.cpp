/**
 * This file is part of the System Settings package
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

#include <qlabel.h>
#include <klocale.h>
#include <kservicegroup.h>
#include <qlayout.h>
#include <kiconloader.h>
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

	QValueList<MenuItem> subMenus = menu->menuList();
 	QValueList<MenuItem>::iterator it;
	for ( it = subMenus.begin(); it != subMenus.end(); ++it ){
		if( !(*it).menu )
			continue;

		// After the first time around add a line
		if( it != subMenus.begin() ){
			QFrame *line = new QFrame( this, "line");
			line->setFrameShadow( QFrame::Sunken );
			line->setFrameShape( QFrame::HLine );
			layout->addWidget( line );
		}

		// Build the row if modueles/icons
		createRow( (*it).subMenu, layout );
	}

	// Make empty iconView for the search widget
	if( groups.count()==0 ) {
		RowIconView *iconView = new RowIconView( this, "groupiconview" );
		iconView->setLineWidth( 0 );
		groups.append( iconView );
	}
	setPaletteBackgroundColor( groups[0]->paletteBackgroundColor() );

	// Align them up!
	{

	uint most = 0;
	QValueList<RowIconView*>::iterator it;
	for ( it = groups.begin(); it != groups.end(); ++it ){
		QIconViewItem *item = (*it)->firstItem();
		while( item ) {
			if(item->width() > most)
				most = item->width();
			item = item->nextItem();
		}
	}

	for ( it = groups.begin(); it != groups.end(); ++it )
		(*it)->setGridX(most);

	}
}

ModulesView::~ModulesView()
{
	if(menu)
		delete menu;
}

void ModulesView::createRow( const QString &parentPath, QBoxLayout *boxLayout )
{
	KServiceGroup::Ptr group = KServiceGroup::group( parentPath );
	if ( !group ){
		kdDebug() << "Invalid Group \"" << parentPath << "\".  Check your installation."<< endl;
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
	QFont textLabel_font(  textLabel->font() );
  textLabel_font.setBold( true );
  textLabel->setFont( textLabel_font );
	rowLayout->addWidget( textLabel );

	boxLayout->addLayout( rowLayout );

	// Make IconView
	RowIconView *iconView = new RowIconView( this, "groupiconview" );
	iconView->setFrameShape( RowIconView::StyledPanel );
	iconView->setLineWidth( 0 );
	iconView->setSpacing( 0 );
	iconView->setMargin( 0 );
	iconView->setItemsMovable( false );
	groups.append( iconView );
	connect(iconView, SIGNAL( clicked( QIconViewItem* ) ),
		      this, SIGNAL( itemSelected( QIconViewItem* ) ) );
	boxLayout->addWidget( iconView );

	// Add all the items in their proper order
	QValueList<MenuItem> list = menu->menuList( parentPath );
 	QValueList<MenuItem>::iterator it;
	for ( it = list.begin(); it != list.end(); ++it ){
		if( !(*it).menu )
			(void)new ModuleIconItem( iconView, (*it).item );
		else
		{
			QString path = (*it).subMenu;
			KServiceGroup::Ptr group = KServiceGroup::group( path );
			if ( group ) {
				ModuleIconItem *item = new ModuleIconItem( ((KIconView*)iconView),
												group->caption(), group->icon() );
				item->modules = menu->modules( path );
			}
		}
	}

	// Force the height for those items that have two words.
	iconView->setMinimumHeight( iconView->minimumSizeHint().height() );
}


#include "modulesview.moc"
