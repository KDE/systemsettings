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
#include <qtimer.h>
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
#include <kactionclasses.h>
#include <ktoolbarbutton.h>
#include <qtabbar.h>

#include "kcmsearch.h"
#include "modulesview.h"
#include "moduleiconitem.h"
#include "kcmodulemenu.h"
#include "kcmultiwidget.h"

MainWindow::MainWindow(bool embed, const QString & menuFile,
								QWidget *parent, const char *name) :
				KMainWindow(parent,name), menu(NULL), embeddedWindows(embed),
				groupWidget(NULL), selectedPage(0), dummyAbout(NULL) {
	
	// Load the menu structure in from disk.
	menu = new KCModuleMenu( menuFile );

	moduleTabs = new KTabWidget(this, "moduletabs",
															QTabWidget::Top|QTabWidget::Rounded);
	buildMainWidget();
	buildActions();
	setupGUI(ToolBar|Save|Create,QString::null);
	widgetChange();
}

MainWindow::~MainWindow()
{
	delete moduleTabs;
	delete windowStack;
	delete menu;	
	delete dummyAbout;
}

void MainWindow::buildMainWidget()
{
	windowStack = new QWidgetStack( this, "widgetstack" );

	// Top level pages.
	QValueList<MenuItem> subMenus = menu->menuList();
	QValueList<MenuItem>::iterator it;
	KCScrollView *modulesScroller;
	moduleTabs->show();
	for ( it = subMenus.begin(); it != subMenus.end(); ++it ) {
		if( (*it).menu ) {
			modulesScroller = new KCScrollView(moduleTabs);
			ModulesView *modulesView = new ModulesView( menu, (*it).subMenu, modulesScroller->viewport(), "modulesView" );
			modulesViewList.append(modulesView);
			connect(modulesView, SIGNAL(itemSelected(QIconViewItem* )), this, SLOT(slotItemSelected(QIconViewItem*)));
			modulesScroller->addChild(modulesView);
			moduleTabs->addTab(modulesScroller, (*it).caption);
			overviewPages.append(modulesScroller);
		}
	}

	windowStack->addWidget(moduleTabs);
	windowStack->raiseWidget(moduleTabs);
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
		showAllAction = new KAction(i18n("Overview"), QApplication::reverseLayout() ? "forward" : "back", 0, this,
								SLOT(showAllModules()), actionCollection(), "showAll" );
		showAllAction->setEnabled(false);
	}

	aboutModuleAction = new KAction(i18n("About Current Module"), 0, this, SLOT(aboutCurrentModule()), actionCollection(), "help_about_module");
	resetModuleHelp();

	// Search
	QHBox *hbox = new QHBox(0);
	hbox->setMaximumWidth( 400 );

	KcmSearch* search = new KcmSearch(&modulesViewList, hbox, "search");
	hbox->setStretchFactor(search,1);
	connect(search, SIGNAL(searchHits(const QString &, int *, int)), this, SLOT(slotSearchHits(const QString &, int *, int)));

	QVBox *vbox = new QVBox(hbox);
	generalHitLabel = new QLabel(vbox);
	vbox->setStretchFactor(generalHitLabel,1);
	advancedHitLabel = new QLabel(vbox);
	vbox->setStretchFactor(advancedHitLabel,1);

	hbox->setStretchFactor(vbox,1);

	// "Search:" label	
	QLabel *searchLabel = new QLabel( this, "SearchLabel");
	searchLabel->setText( i18n("&Search:") );
	searchLabel->setFont(KGlobalSettings::toolBarFont());
	searchLabel->setMargin(2);
	searchText = new KWidgetAction( searchLabel, i18n("&Search:"), Key_F6, 0, 0, actionCollection(), "searchText" );
	searchLabel->setBuddy( search );

	// The search box.
	searchAction = new KWidgetAction( hbox, i18n( "Search System Settings" ), 0,
                  0, 0, actionCollection(), "search" );
	searchAction->setShortcutConfigurable( false );
	searchAction->setAutoSized( true );
	QWhatsThis::add( search, i18n( "Search Bar<p>Enter a search term." ) );

	// The Clear search box button.
	KToolBarButton *clearWidget = new KToolBarButton(QApplication::reverseLayout() ? "clear_left" : "locationbar_erase",
		0, this);
	searchClear = new KWidgetAction( clearWidget, QString(""), CTRL+Key_L, search, SLOT(clear()),
					actionCollection(), "searchReset");
	connect(clearWidget, SIGNAL(clicked()), searchClear, SLOT(activate()));
	searchClear->setWhatsThis( i18n( "Reset Search\n"
                                        "Resets the search so that "
                                        "all items are shown again." ) );

	// Top level pages.
	QValueList<MenuItem> subMenus = menu->menuList();
	QValueList<MenuItem>::iterator it;
	for ( it = subMenus.begin(); it != subMenus.end(); ++it ) {
		if( (*it).menu ) {
			KServiceGroup::Ptr group = KServiceGroup::group( (*it).subMenu );
			if ( !group ){
				kdDebug() << "Invalid Group \"" << (*it).subMenu << "\".  Check your installation."<< endl;
				continue;
			}

			KRadioAction *newAction = new KRadioAction( group->caption(), group->icon(), KShortcut(), this,
				SLOT(slotTopPage()), actionCollection(), group->relPath() );
			pageActions.append(newAction);
kdDebug() << "relpath is :" << group->relPath() << endl;
		}
	}
	pageActions.at(0)->setChecked(true);
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
	showAllModules();
}

void MainWindow::showAllModules()
{
	windowStack->raiseWidget(moduleTabs);

	// Reset the widget for normal all widget viewing
	groupWidget = 0;
	widgetChange();

	if( embeddedWindows ) {
		showAllAction->setEnabled(false);
	}
	aboutModuleAction->setEnabled(false);

	searchText->setEnabled(true);
	searchClear->setEnabled(true);
	searchAction->setEnabled(true);

	KRadioAction *currentRadioAction;
	for ( currentRadioAction = pageActions.first(); currentRadioAction; currentRadioAction = pageActions.next()) {
		currentRadioAction->setEnabled(true);
	}

	resetModuleHelp();
}

void MainWindow::slotItemSelected( QIconViewItem *item ){
	ModuleIconItem *mItem = (ModuleIconItem *)item;

	if( !mItem )
		return;

	groupWidget = moduleItemToWidgetDict.find(mItem);
	scrollView = moduleItemToScrollerDict.find(mItem);

	if(groupWidget==0) {
		QValueList<KCModuleInfo> list = mItem->modules;
		KDialogBase::DialogType type = KDialogBase::IconList;
		if(list.count() == 1) {
			type=KDialogBase::Plain;
		}

		scrollView = new KCScrollView(windowStack);
		groupWidget = new KCMultiWidget(type, scrollView->viewport(), "moduleswidget");
                scrollView->addChild(groupWidget);
		windowStack->addWidget(scrollView);
		moduleItemToScrollerDict.insert(mItem,scrollView);
		moduleItemToWidgetDict.insert(mItem,groupWidget);

		connect(groupWidget, SIGNAL(aboutToShow( KCModuleProxy * )), this, SLOT(updateModuleHelp( KCModuleProxy * )));
		connect(groupWidget, SIGNAL(aboutToShowPage( QWidget* )), this, SLOT(widgetChange()));
		connect(groupWidget, SIGNAL(finished()), this, SLOT(groupModulesFinished()));
		connect(groupWidget, SIGNAL(close()), this, SLOT(showAllModules()));

		QValueList<KCModuleInfo>::iterator it;
		for ( it = list.begin(); it != list.end(); ++it ){
			qDebug("adding %s %s", (*it).moduleName().latin1(), (*it).fileName().latin1());
			groupWidget->addModule(	*it );
		}
		groupWidget->reparent(scrollView->viewport(), 0, QPoint());
		scrollView->reparent(windowStack, 0, QPoint());
	}

	if( embeddedWindows ) {
		windowStack->raiseWidget( scrollView );

		setCaption( mItem->text() );
		showAllAction->setEnabled(true);
		searchText->setEnabled(false);
		searchClear->setEnabled(false);
		searchAction->setEnabled(false);

		KRadioAction *currentRadioAction;
		for ( currentRadioAction = pageActions.first(); currentRadioAction; currentRadioAction = pageActions.next()) {
			currentRadioAction->setEnabled(false);
		}

	} else {
		scrollView->show();
	}
	groupWidget->show();

	// We resize and expand the window if neccessary, but only once the window has been updated.
	// Some modules seem to dynamically change thier size. The new size is only available
	// once the dialog is updated. :-/ -SBE
	QTimer::singleShot(0,this,SLOT(timerResize()));
}

void MainWindow::timerResize() {
	QSize currentSize = size();
	QSize newSize = currentSize.expandedTo(sizeHint());
	// Avoid resizing if possible.
	if(newSize!=currentSize) {
		resize(newSize);
	}
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
	QString name;
	if( groupWidget && groupWidget->currentModule()) {
		name = groupWidget->currentModule()->moduleInfo().moduleName();
	}

	if( !groupWidget ) {
		setCaption( "" );
		
		ModulesView *modulesView;
		for( modulesView = modulesViewList.first(); modulesView; modulesView = modulesViewList.next()) {
			modulesView->clearSelection();
		}
	}
}

void MainWindow::slotTopPage() {
	KRadioAction *clickedRadioAction = (KRadioAction *)sender();
	selectedPage = pageActions.find(clickedRadioAction);

	KRadioAction *currentRadioAction;
    for ( currentRadioAction = pageActions.first(); currentRadioAction; currentRadioAction = pageActions.next()) {
		currentRadioAction->setChecked(currentRadioAction==clickedRadioAction);
	}

	windowStack->raiseWidget(overviewPages.at(selectedPage));
}

void MainWindow::slotSearchHits(const QString &query, int *hitList, int length) {
	if(query=="") {
		generalHitLabel->setText("");
		advancedHitLabel->setText("");
	} else {
		
		if(length>=1) {
			generalHitLabel->setText(i18n("%1 hit in General","%1 hits in General",hitList[0]).arg(hitList[0]));
		}
	
		if(length>=2) {
			advancedHitLabel->setText(i18n("%1 hit in Advanced","%1 hits in Advanced",hitList[1]).arg(hitList[1]));
		}

	}
}

#include "mainwindow.moc"
