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

#include "mainwindow.h"

#include <kstdaction.h>
#include <qwhatsthis.h>
#include <qlabel.h>
#include <qvbox.h>
#include <kaction.h>
#include <qtoolbutton.h>
#include <klocale.h>
#include <kservicegroup.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <kiconloader.h>
#include <kcmoduleloader.h>
#include <kdialogbase.h>
#include <kiconviewsearchline.h>
#include <kapplication.h>
#include <kaboutapplication.h>
#include <kdebug.h>
#include <kcmoduleproxy.h>
#include <kcmultidialog.h>
#include <kbugreport.h>

#include "kcmsearch.h"
#include "modulesview.h"
#include "moduleiconitem.h"
#include "kcmodulemenu.h"
#include "kcmultiwidget.h"

MainWindow::MainWindow(QWidget *parent, const char *name) :
				KMainWindow(parent,name),
				reportBugAction(NULL) {
	buildActions();
	setupGUI();
	buildMainWidget();

	// Steal the report bug
	reportBugAction = actionCollection()->action("help_report_bug");
	if(reportBugAction)
	{
		reportBugAction->disconnect();
  		connect(reportBugAction, SIGNAL(activated()), SLOT(reportBug()));
	}
}

MainWindow::~MainWindow()
{
}

void MainWindow::buildMainWidget()
{
	windowStack = new QWidgetStack( this, "widgetstack" );
	KCModuleMenu *mainMenu = new KCModuleMenu( "systempreferences" );

	rootView = new ModulesView(mainMenu, windowStack);
	rootView->createRow( mainMenu->submenus() );

	int id = 1;
	windowStack->addWidget(rootView, id);
	connect( rootView, SIGNAL(itemSelected(QIconViewItem*)),
		this, SLOT(slotRootItemSelected(QIconViewItem*)) );

	// Search
	KcmSearch* search = new KcmSearch(rootView, 0);
	connect( search, SIGNAL( textChanged( const QString & ) ), this, SLOT( slotSearchAll( const QString & ) ) );
	KWidgetAction *searchAction = new KWidgetAction( search, i18n( "Search System Preferences" ), 0, 0, 0, actionCollection(), 0 );
	searchAction->setShortcutConfigurable( false );
	searchAction->setAutoSized( true );
	searchAction->plug( toolBar("mainToolBar") );
	QWhatsThis::add( search, i18n( "Search Bar<p>Enter a search term." ) );

	searchActions.insert( rootView, searchAction );
	searchers.insert( rootView, search );
	searchLabel->setBuddy( search );

	QStringList subMenus = mainMenu->submenus();
	for( QStringList::ConstIterator it = subMenus.begin(); it != subMenus.end(); ++it )
	{
		ModulesView *modulesView = new ModulesView(mainMenu,windowStack);
		modulesView->createRow( *it );
		windowStack->addWidget(modulesView, ++id);

		connect(modulesView, SIGNAL(itemSelected(QIconViewItem* )),
			this, SLOT(slotItemSelected(QIconViewItem*)));

		// Search
		KcmSearch* search = new KcmSearch(modulesView, 0);
		connect( search, SIGNAL( textChanged( const QString & ) ), this, SLOT( slotSearchAll( const QString & ) ) );
		KWidgetAction *searchAction = new KWidgetAction( search, i18n( "Search System Preferences" ), 0, 0, 0, actionCollection(), 0 );
		searchAction->setShortcutConfigurable( false );
		searchAction->setAutoSized( true );
		QWhatsThis::add( search, i18n( "Search Bar<p>Enter a search term." ) );

		searchActions.insert( modulesView, searchAction );
		searchers.insert( modulesView, search );
	}

	setCentralWidget(windowStack);
	windowStack->raiseWidget(0);
}

void MainWindow::buildActions()
{
	KStdAction::quit(this, SLOT( close() ), actionCollection());

	resetModule = new KAction(i18n("Undo changes"), 0, this, SLOT(showAllModules()), actionCollection(), "resetModule" );
	resetModule->setEnabled(false);

	defaultModule = new KAction(i18n("Reset to defaults"), 0, this, SLOT(showAllModules()), actionCollection(), "defaultModule" );
	defaultModule->setEnabled(false);

	showAllAction = new KAction(i18n("Show &All"), 0, this, SLOT(showAllModules()), actionCollection(), "showAll" );
	showAllAction->setEnabled(false);

	searchLabel = new QLabel( this, "SearchLabel");
	searchLabel->setText( i18n("&Search: ") );
	searchLabel->setMargin(2);
	searchText = new KWidgetAction( searchLabel, i18n("&Search: "), Key_F6, 0, 0, actionCollection(), "searchText" );

	searchClear = new KAction( i18n( "Reset" ),
                                           QApplication::reverseLayout()
                                           ? "clear_left"
                                           : "locationbar_erase",
                                           CTRL+Key_L, this, SLOT(slotClearSearch()),
                                           actionCollection(),
                                           "searchReset" );
	searchClear->plug( toolBar("mainToolBar") );

	searchClear->setWhatsThis( i18n( "Reset Search\n"
                                        "Resets the search so that "
                                        "all itmes are shown again." ) );
}

/**
 * From the old KControl module.
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 * Copyright (c) 2000 Matthias Elter <elter@kde.org>
 */
void MainWindow::reportBug()
{
	// this assumes the user only opens one bug report at a time
	KBugReport *br = new KBugReport(this, false);
	br->show();
}

void MainWindow::slotClearSearch()
{
	searchers[ windowStack->visibleWidget() ]->clear();
}

void MainWindow::slotSearchAll( const QString &text )
{
	for( QMap<QWidget*,KcmSearch*>::iterator it = searchers.begin(); it != searchers.end(); ++it )
	{
		if( it.key() != windowStack->visibleWidget() )
		{
			it.data()->setText( text );
			it.data()->updateSearch();
		}
	}
}

void MainWindow::showAllModules()
{
	searchActions[ windowStack->visibleWidget() ]->unplug( toolBar("mainToolBar") );
	windowStack->raiseWidget(1);
        searchActions[ rootView ]->plug( toolBar("mainToolBar") );
	showAllAction->setEnabled(false);
	searchLabel->setBuddy( searchActions[ rootView ]->widget() );
}

void MainWindow::slotItemSelected( QIconViewItem *item )
{
	ModuleIconItem *mItem = (ModuleIconItem *)item;
	if( !mItem )
		return;

 	QValueList<KCModuleInfo> list = mItem->modules;
	KDialogBase::DialogType type = KDialogBase::IconList;
	if(list.count() == 1)
		type=KDialogBase::Plain;

	KCMultiDialog *widgetDialog = new KCMultiDialog((int)type, item->text(), this, "moduleswidget");
	widgetDialog->setIcon( *(item->pixmap()) );

	QValueList<KCModuleInfo>::iterator it;
	for ( it = list.begin(); it != list.end(); ++it ){
		qDebug("adding %s %s", (*it).moduleName().latin1(), (*it).fileName().latin1());
		widgetDialog->addModule(*it);
	}

	widgetDialog->show();
	KDialog::centerOnScreen(widgetDialog);
}

void MainWindow::slotRootItemSelected( QIconViewItem *item )
{
	if( item )
	{
		searchActions[ rootView ]->unplug( toolBar("mainToolBar") );
		windowStack->raiseWidget( item->index() + 2 );
		searchActions[ windowStack->visibleWidget() ]->plug( toolBar("mainToolBar") );
		showAllAction->setEnabled(true);
		searchLabel->setBuddy( searchActions[ windowStack->visibleWidget() ]->widget() );
	}
}


