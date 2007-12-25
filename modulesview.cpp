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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "modulesview.h"

#include <qlabel.h>
#include <QListWidget>
#include <qlayout.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kservicetypetrader.h>
#include <QApplication>
#include <kdialog.h>

#include "kcmsearch.h"
#include "moduleiconitem.h"
#include "kcmodulemenu.h"

ModulesView::ModulesView( KCModuleMenu *rootMenu, const QString &menuPath, QWidget *parent ) : QWidget( parent ), rootMenu( NULL )
{
	this->rootMenu = rootMenu;	
	this->menuPath = menuPath;
	this->categories = KServiceTypeTrader::self()->query("SystemSettingsCategory");

	QVBoxLayout *layout = new QVBoxLayout( this );
	layout->setMargin( KDialog::marginHint() );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setObjectName( QLatin1String( "layout" ) );

	displayName = this->rootMenu->caption;

	QList<MenuItem> subMenus = rootMenu->menuList(menuPath);
	QList<MenuItem>::const_iterator it;
	for ( it = subMenus.begin(); it != subMenus.end(); ++it ){
		if( !(*it).menu )
			continue;

		// After the first time around add a line
		if( it != subMenus.begin() ){
			QFrame *line = new QFrame( this );
			line->setObjectName( QLatin1String( "line" ) );
			line->setFrameShadow( QFrame::Sunken );
			line->setFrameShape( QFrame::HLine );
			layout->addWidget( line );
		}

		// Build the row of modules/icons
		createRow( (*it).subMenu, layout );
	}
	layout->addStretch(1);

	setBackgroundRole(QPalette::Base);
	setForegroundRole(QPalette::Text);

	/*
	// Align them up!
	uint most = 0;
	QList<RowIconView*>::iterator it2;
	for ( it2 = groups.begin(); it2 != groups.end(); ++it2 ){
		for (int i = 0; i < (*it2)->count(); ++i ) {
			QListWidgetItem * item = (*it2)->item( i );
			if ( item && item->sizeHint().width() > most ) {
				most = item->sizeHint().width();
			}
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
	connect(iconWidget, SIGNAL( itemClicked( QListWidgetItem* ) ),
		      this, SIGNAL( itemSelected( QListWidgetItem* ) ) );
	connect(iconWidget, SIGNAL( itemActivated( QListWidgetItem* ) ),
		      this, SIGNAL( itemSelected( QListWidgetItem* ) ) );
	groups.append( iconWidget );
	boxLayout->addWidget( iconWidget );

	int height = 0;
	// Add all the items in their proper order
	QList<MenuItem> list = rootMenu->menuList( parentPath );
	QList<MenuItem>::const_iterator it;
	for ( it = list.begin(); it != list.end(); ++it ){
		ModuleIconItem *item = NULL;
		if( !(*it).menu ) {
			item = new ModuleIconItem( iconWidget, (*it).item );
		} else {
			QString path = (*it).subMenu;

			QString categoryCaption = (*it).caption;
			QString iconFile;
			for (int i = 0; i < categories.size(); ++i) {
				const KService* entry = categories.at(i).data();
				if (entry->name() == categoryCaption) {
					iconFile = entry->icon();
					break;
				}
			}

			const QList<KCModuleInfo> &modules = rootMenu->modules( path );
			if ( modules.count() > 0 ) {
				item = new ModuleIconItem( iconWidget, categoryCaption, iconFile);
				item->modules = modules;
			}
		}
		if (item) height = qMax(height, item->data(Qt::SizeHintRole).toSize().height());
	}

	// give the proper height to make all the items visible
	iconWidget->setMinimumHeight(height);
}

void ModulesView::clearSelection() {
	QList<RowIconView*>::const_iterator it;
	for ( it = groups.begin(); it != groups.end(); ++it ) {
		(*it)->clearSelection();
	}
}

#include "modulesview.moc"
