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
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "modulesview.h"

#include <qlabel.h>
//Added by qt3to4:
#include <QListWidget>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <Q3ValueList>
#include <Q3Frame>
#include <klocale.h>
#include <kservicegroup.h>
#include <qlayout.h>
#include <kiconloader.h>
#include <kcmultidialog.h>
#include <kapplication.h>
#include <kdebug.h>
#include <q3iconview.h>
#include <k3iconview.h>
#include <kservicetypetrader.h>

#include "kcmsearch.h"
#include "moduleiconitem.h"
#include "kcmodulemenu.h"

ModulesView::ModulesView( KCModuleMenu *rootMenu, const QString &menuPath, QWidget *parent,
		const char *name ) : QWidget( parent, name ), rootMenu( NULL )
{
	this->rootMenu = rootMenu;	
	this->menuPath = menuPath;

	Q3VBoxLayout *layout = new Q3VBoxLayout( this, 11, 6, "layout" );

	displayName = this->rootMenu->caption;

	Q3ValueList<MenuItem> subMenus = rootMenu->menuList(menuPath);
 	Q3ValueList<MenuItem>::iterator it;
	for ( it = subMenus.begin(); it != subMenus.end(); ++it ){
		if( !(*it).menu )
			continue;

		// After the first time around add a line
		if( it != subMenus.begin() ){
			Q3Frame *line = new Q3Frame( this, "line");
			line->setFrameShadow( Q3Frame::Sunken );
			line->setFrameShape( Q3Frame::HLine );
			line->setMargin(0);
			layout->addWidget( line );
		}

		// Build the row of modules/icons
		createRow( (*it).subMenu, layout );
	}
	layout->addStretch(1);

	// Make empty iconView for the search widget
	oldIconView = new K3IconView(this, "foo");
	oldIconView->hide();

	// set background colour to the icon row background colour
	setAutoFillBackground(true);
	QPalette rowPalette = oldIconView->palette();
	QColor background = rowPalette.color(QPalette::Base);
	QPalette palette;
	palette.setColor(backgroundRole(), background);
	setPalette(palette);

	// Align them up!
/*FIXME
	{
	uint most = 0;
	Q3ValueList<RowIconView*>::iterator it;
	for ( it = groups.begin(); it != groups.end(); ++it ){
		Q3IconViewItem *item = (*it)->firstItem();
		while( item ) {
			if(item->width() > most)
				most = item->width();
			item = item->nextItem();
		}
	}
*/
/*FIXME
	for ( it = groups.begin(); it != groups.end(); ++it )
		(*it)->setGridX(most);

	}
*/
}

ModulesView::~ModulesView()
{
}

void ModulesView::createRow( const QString &parentPath, Q3BoxLayout *boxLayout )
{
	//find the category name and search for it
	QString categoryName = parentPath.section('/', -2, -2);
	KService::List categories = KServiceTypeTrader::self()->query("SystemSettingsCategory");
	QString iconName;
	for (int i = 0; i < categories.size(); ++i) {
		const KService* entry = categories.at(i).data();
		if (entry->name() == categoryName) {
			iconName = entry->icon();
			break;
		}
	}

	// Make header
	Q3HBoxLayout *rowLayout = new Q3HBoxLayout( 0, 0, 6, "rowLayout" );

	// Header Icon
	QLabel *icon = new QLabel( this, "groupicon" );
	icon->setPixmap( SmallIcon( iconName ));
	icon->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1,
		(QSizePolicy::SizeType)5, 0, 0, icon->sizePolicy().hasHeightForWidth() ) );
	rowLayout->addWidget( icon );

	// Header Name
	QLabel *textLabel = new QLabel( this, "groupcaption" );
	textLabel->setText( categoryName );
	textLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7,
		(QSizePolicy::SizeType)5, 0, 0, textLabel->sizePolicy().hasHeightForWidth()));
	QFont textLabel_font(  textLabel->font() );
	textLabel_font.setBold( true );
	textLabel->setFont( textLabel_font );
	rowLayout->addWidget( textLabel );

	boxLayout->addLayout( rowLayout );

	// Make IconView
	RowIconView* iconWidget = new RowIconView( this );
	iconWidget->setViewMode(QListView::IconMode);
	iconWidget->setMovement(QListWidget::Static);
	iconWidget->setFrameShape( RowIconView::NoFrame );
	iconWidget->setLineWidth(0);
	iconWidget->setSpacing(0);
	iconWidget->setWordWrap(true);//FIXME why doesn't this work?
	iconWidget->setGridSize(QSize(100, 48));
	connect(iconWidget, SIGNAL( itemClicked( QListWidgetItem* ) ),
		      this, SIGNAL( itemSelected( QListWidgetItem* ) ) );
	connect(iconWidget, SIGNAL( itemActivated( QListWidgetItem* ) ),
		      this, SIGNAL( itemSelected( QListWidgetItem* ) ) );
	groups.append( iconWidget );
	boxLayout->addWidget( iconWidget );

	// Add all the items in their proper order
	Q3ValueList<MenuItem> list = rootMenu->menuList( parentPath );
 	Q3ValueList<MenuItem>::iterator it;
	for ( it = list.begin(); it != list.end(); ++it ){
		if( !(*it).menu ) {
			(void)new ModuleIconItem( iconWidget, (*it).item );
		} else {
			QString path = (*it).subMenu;

			QString categoryCaption = path.section('/', -2, -2);
			QString iconFile;
			for (int i = 0; i < categories.size(); ++i) {
				const KService* entry = categories.at(i).data();
				if (entry->name() == categoryCaption) {
					iconFile = entry->icon();
					break;
				}
			}

			if ( ! iconFile.isEmpty() ) {
				ModuleIconItem *item = new ModuleIconItem( iconWidget, categoryCaption, iconFile);
				item->modules = rootMenu->modules( path );
			}
		}
	}

	// Force the height for those items that have two words.
	iconWidget->setMaximumHeight(iconWidget->minimumSizeHint().height());
}

void ModulesView::clearSelection() {
	Q3ValueList<RowIconView*>::iterator it;
	for ( it = groups.begin(); it != groups.end(); ++it ) {
		(*it)->clearSelection();
	}
}

#include "modulesview.moc"
