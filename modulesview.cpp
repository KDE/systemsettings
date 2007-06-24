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
#include <QListWidget>
#include <qlayout.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kservicetypetrader.h>
#include <Q3ValueList>
#include <QApplication>
#include <Q3Frame>

#include "kcmsearch.h"
#include "moduleiconitem.h"
#include "kcmodulemenu.h"

ModulesView::ModulesView( KCModuleMenu *rootMenu, const QString &menuPath, QWidget *parent,
		const char *name ) : QWidget( parent ), rootMenu( NULL )
{
	this->rootMenu = rootMenu;	
	this->menuPath = menuPath;

	QVBoxLayout *layout = new QVBoxLayout( this );
	layout->setMargin( 11 );
	layout->setSpacing( 6 );
	layout->setObjectName( QLatin1String( "layout" ) );

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

	// set background colour to the icon row background colour
	setAutoFillBackground(true);
	QPalette rowPalette = QApplication::palette();
	QColor background = rowPalette.color(QPalette::Base);
	QPalette palette;
	palette.setColor(backgroundRole(), background);
	setPalette(palette);

	// Align them up!
/*FIXME
	{
	uint most = 0;
	QList<RowIconView*>::iterator it;
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

void ModulesView::createRow( const QString &parentPath, QBoxLayout *boxLayout )
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
	QHBoxLayout *rowLayout = new QHBoxLayout();
	rowLayout->setMargin( 0 );
	rowLayout->setSpacing( 6 );
	rowLayout->setObjectName( QLatin1String( "rowLayout" ) );

	// Header Icon
	QLabel *icon = new QLabel( this );
	icon->setObjectName( QLatin1String( "groupicon" ) );
	icon->setPixmap( SmallIcon( iconName ));
	QSizePolicy sp( QSizePolicy::Minimum, QSizePolicy::Preferred );
	sp.setHeightForWidth( icon->sizePolicy().hasHeightForWidth() );
	icon->setSizePolicy( sp );
	rowLayout->addWidget( icon );

	// Header Name
	QLabel *textLabel = new QLabel( this );
	textLabel->setObjectName( QLatin1String( "groupcaption" ) );
	textLabel->setText( categoryName );
	QSizePolicy sp1( QSizePolicy::Expanding, QSizePolicy::Preferred );
	sp1.setHeightForWidth( textLabel->sizePolicy().hasHeightForWidth() );
	textLabel->setSizePolicy( sp1 );
	QFont textLabel_font(  textLabel->font() );
	textLabel_font.setBold( true );
	textLabel->setFont( textLabel_font );
	rowLayout->addWidget( textLabel );

	boxLayout->addLayout( rowLayout );

	// Make IconView
	RowIconView* iconWidget = new RowIconView( this );
	iconWidget->setFlow(QListView::LeftToRight);
	iconWidget->setResizeMode(QListView::Adjust);
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

	// give the proper height to make all the items visible
	// TODO: this should be done whenever the mainwindow is resized (eg dynamically adapt to its new size)
	QRect r = iconWidget->visualItemRect(iconWidget->item(iconWidget->count() - 1));
	iconWidget->setMaximumHeight(r.bottom());
}

void ModulesView::clearSelection() {
	QList<RowIconView*>::const_iterator it;
	for ( it = groups.begin(); it != groups.end(); ++it ) {
		(*it)->clearSelection();
	}
}

#include "modulesview.moc"
