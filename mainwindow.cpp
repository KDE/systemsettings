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
#include <kbugreport.h>
#include <kmenubar.h>

#include "kcmsearch.h"
#include "modulesview.h"
#include "moduleiconitem.h"
#include "kcmodulemenu.h"
#include "kcmultiwidget.h"

MainWindow::MainWindow(bool embed, const QString & menuFile,
								QWidget *parent, const char *name) :
				KMainWindow(parent,name), embeddedWindows(embed), groupWidget(NULL),
				reportBugAction(NULL), dummyAbout(NULL) {
	buildMainWidget( menuFile );
	buildActions();
	setupGUI();
	menuBar()->hide();

	// Steal the report bug
	reportBugAction = actionCollection()->action("help_report_bug");
  if(reportBugAction){
		reportBugAction->disconnect();
	  connect(reportBugAction, SIGNAL(activated()), SLOT(reportBug()));
	}

	widgetChange();
}

MainWindow::~MainWindow()
{
	delete dummyAbout;
}

void MainWindow::buildMainWidget( const QString &menuFile )
{
	windowStack = new QWidgetStack( this, "widgetstack" );
	modulesView = new ModulesView( menuFile, windowStack, "modulesView" );
	windowStack->addWidget(modulesView);

	connect(modulesView, SIGNAL(itemSelected(QIconViewItem* )), this, SLOT(slotItemSelected(QIconViewItem*)));
	setCentralWidget(windowStack);
}

void MainWindow::buildActions()
{
	KStdAction::quit(this, SLOT( close() ), actionCollection());

	resetModule = new KAction(i18n("Undo Changes"), 0, this,
								SLOT(showAllModules()), actionCollection(), "resetModule" );
	resetModule->setEnabled(false);

	defaultModule = new KAction(i18n("Reset to Defaults"), 0, this,
								SLOT(showAllModules()), actionCollection(), "defaultModule" );
	defaultModule->setEnabled(false);

	if( embeddedWindows ) {
		showAllAction = new KAction(i18n("Show &All"), QApplication::reverseLayout() ? "forward" : "back", 0, this,
								SLOT(showAllModules()), actionCollection(), "showAll" );
		showAllAction->setEnabled(false);
	}

	aboutModuleAction = new KAction(i18n("About Current Module"), 0, this, SLOT(aboutCurrentModule()), actionCollection(), "help_about_module");
	resetModuleHelp();

	// Search
	KcmSearch* search = new KcmSearch(modulesView, 0, "search");
	search->setMaximumWidth( 200 );

	/*
	// Move to the right
	KToolBar *bar = (KToolBar *)widget;
  int id_ = getToolButtonID();
  bar->alignItemRight( id_ );
	*/

	QLabel *searchLabel = new QLabel( this, "SearchLabel");
	searchLabel->setText( i18n("&Search:") );
	//searchLabel->setPixmap( SmallIcon("find"));
	searchLabel->setMargin(2);
	searchText = new KWidgetAction( searchLabel, i18n("&Search:"), Key_F6, 0, 0, actionCollection(), "searchText" );
	searchLabel->setBuddy( search );

	searchAction = new KWidgetAction( search, i18n( "Search System Settings" ), 0,
                  0, 0, actionCollection(), "search" );
	searchAction->setShortcutConfigurable( false );
	searchAction->setAutoSized( true );
	QWhatsThis::add( search, i18n( "Search Bar<p>"
													        "Enter a search term." ) );

	searchClear = new KAction( i18n( "Reset" ),
                                           QApplication::reverseLayout()
                                           ? "clear_left"
                                           : "locationbar_erase",
                                           CTRL+Key_L, search, SLOT(clear()),
                                           actionCollection(),
                                           "searchReset" );

	searchClear->setWhatsThis( i18n( "Reset Search\n"
                                        "Resets the search so that "
                                        "all items are shown again." ) );
}

/**
 * From the old KControl module.
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 * Copyright (c) 2000 Matthias Elter <elter@kde.org>
 */
void MainWindow::reportBug()
{
	// this assumes the user only opens one bug report at a time
  static char buffer[128];

  dummyAbout = 0;
  bool deleteit = false;

	if (!groupWidget || !groupWidget->currentModule()) // report against kcontrol
		dummyAbout = const_cast<KAboutData*>(KGlobal::instance()->aboutData());
	else
	{
		if (groupWidget->currentModule()->aboutData())
			dummyAbout = const_cast<KAboutData*>(groupWidget->currentModule()->aboutData());
		else
		{
			snprintf(buffer, sizeof(buffer), "kcm%s", groupWidget->currentModule()->moduleInfo().library().latin1());
			dummyAbout = new KAboutData(buffer, groupWidget->currentModule()->moduleInfo().moduleName().utf8(), "2.0");
			deleteit = true;
		}
	}
	KBugReport *br = new KBugReport(this, false, dummyAbout);
	if (deleteit)
		connect(br, SIGNAL(finished()), SLOT(deleteDummyAbout()));
	else
		dummyAbout = 0;
	br->show();

}

void MainWindow::aboutCurrentModule()
{
	if(!groupWidget)
		return;

	KCModuleProxy* module = groupWidget->currentModule();
	if( module && module->aboutData() ){
		KAboutApplication dlg( module->aboutData() );
		dlg.exec();
	}
}

void MainWindow::groupModulesFinished()
{
	windowStack->removeWidget( groupWidget );
	groupWidget = NULL;
	showAllModules();
}

void MainWindow::showAllModules()
{
	windowStack->raiseWidget(modulesView);

	if( groupWidget ){
		windowStack->removeWidget( groupWidget );
		groupWidget->removeAllModules();
		groupWidget->close(true);
		groupWidget = NULL;
	}
	else
		kdDebug() << "No group widget." << endl;

	// Wait for the widget to be removed from the parent before resizing
	qApp->processEvents();

	// Reset the widget for normal all widget viewing
	widgetChange();

	if( embeddedWindows )
		showAllAction->setEnabled(false);
	aboutModuleAction->setEnabled(false);

	searchText->setEnabled(true);
	searchClear->setEnabled(true);
	searchAction->setEnabled(true);

	resetModuleHelp();
}

void MainWindow::slotItemSelected( QIconViewItem *item ){
	ModuleIconItem *mItem = (ModuleIconItem *)item;
	if( !mItem )
		return;

 	QValueList<KCModuleInfo> list = mItem->modules;
	KDialogBase::DialogType type = KDialogBase::IconList;
	if(list.count() == 1)
		type=KDialogBase::Plain;
	groupWidget = new KCMultiWidget(type, windowStack, "moduleswidget");
	connect(groupWidget, SIGNAL(aboutToShow( KCModuleProxy * )), this, SLOT(updateModuleHelp( KCModuleProxy * )));
	connect(groupWidget, SIGNAL(aboutToShowPage( QWidget* )), this, SLOT(widgetChange()));
	connect(groupWidget, SIGNAL(finished()), this, SLOT(groupModulesFinished()));

	QValueList<KCModuleInfo>::iterator it;
	for ( it = list.begin(); it != list.end(); ++it ){
		qDebug("adding %s %s", (*it).moduleName().latin1(), (*it).fileName().latin1());
		groupWidget->addModule(	*it );
	}

	if( embeddedWindows ) {
		groupWidget->reparent(windowStack, 0, QPoint());
		int stackId = windowStack->addWidget(groupWidget);
		windowStack->raiseWidget( stackId );
		setCaption( mItem->text() );
		resize( minimumSizeHint() );
		showAllAction->setEnabled(true);
		searchText->setEnabled(false);
		searchClear->setEnabled(false);
		searchAction->setEnabled(false);
	}
	else
		groupWidget->show();
}

void MainWindow::updateModuleHelp( KCModuleProxy *currentModule ) {
	if ( currentModule->aboutData() ) {
		aboutModuleAction->setText(i18n("Help menu->about <modulename>", "About %1").arg(
				                             currentModule->moduleInfo().moduleName().replace("&","&&")));
		aboutModuleAction->setIcon(currentModule->moduleInfo().icon());
		aboutModuleAction->setEnabled(true);
	}
	else {
		resetModuleHelp();
	}
}

void MainWindow::resetModuleHelp() {
	aboutModuleAction->setText(i18n("About Current Module"));
	aboutModuleAction->setIconSet(QIconSet());
	aboutModuleAction->setEnabled(false);
}

void MainWindow::widgetChange() {
	resize( minimumSizeHint() );
	QString name;
	if( groupWidget && groupWidget->currentModule())
		name = groupWidget->currentModule()->moduleInfo().moduleName();

	if( !groupWidget )
		setCaption( "" );

	if ( !reportBugAction )
		return;
  if( name.isEmpty() )
		reportBugAction->setText(i18n("&Report Bug..."));
  else
		reportBugAction->setText(i18n("Report Bug on Module %1...").arg( name.replace("&","&&")));
}


#include "mainwindow.moc"
