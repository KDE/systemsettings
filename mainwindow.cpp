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

#include "mainwindow.h"

#include <QIcon>
#include <kstandardaction.h>
#include <ktoggletoolbaraction.h>
#include <ktoolbarspaceraction.h>
#include <kaboutapplicationdialog.h>
#include <QLabel>
#include <QStackedWidget>
#include <QListWidgetItem>
#include <kaction.h>
#include <klocale.h>
#include <kservicegroup.h>
#include <qlayout.h>
#include <qtimer.h>
#include <kiconloader.h>
#include <kcmoduleloader.h>
#include <kactioncollection.h>
#include <qapplication.h>
#include <kdebug.h>
#include <kcmoduleproxy.h>
#include <kbugreport.h>
#include <kmenubar.h>
#include <ktoggleaction.h>
#include <qscrollarea.h>
#include <kcmoduleinfo.h>
#include <ktabwidget.h>
#include <kaboutdata.h>

#include "kcmsearch.h"
#include "modulesview.h"
#include "moduleiconitem.h"
#include "kcmodulemenu.h"
#include "kcmultiwidget.h"

MainWindow::MainWindow(const QString & menuFile, QWidget *parent) :
				KXmlGuiWindow(parent), menu(NULL),
				groupWidget(NULL), selectedPage(0), dummyAbout(NULL) {

	// Load the menu structure in from disk.
	menu = new KCModuleMenu( menuFile );

	moduleTabs = new KTabWidget(this, QTabWidget::North|QTabWidget::Rounded);
	buildMainWidget();
	buildActions();
	setupGUI(ToolBar|Save|Create,QString());
	widgetChange();
	menuBar()->hide();
}

MainWindow::~MainWindow()
{
	delete moduleTabs;
	delete menu;
	delete dummyAbout;
}

void MainWindow::closeEvent ( QCloseEvent * )
{
	if ( groupWidget ) {
	        groupWidget->dialogClosed();
	}
}


void MainWindow::buildMainWidget()
{
	windowStack = new QStackedWidget(this);

	// Top level pages.
	QList<MenuItem> subMenus = menu->menuList();
	QList<MenuItem>::iterator it;
	QScrollArea *modulesScroller;
	moduleTabs->show();

	foreach( const MenuItem &item , subMenus ) {
		if( item.menu ) {
			modulesScroller = new QScrollArea(moduleTabs);

			modulesScroller->setFrameStyle( QFrame::NoFrame );

			modulesScroller->setWidgetResizable(true);
			ModulesView *modulesView = new ModulesView( menu, item.subMenu, modulesScroller );
			modulesView->setObjectName(QLatin1String("modulesView"));
			modulesViewList.append(modulesView);
			connect(modulesView, SIGNAL(itemSelected(QListWidgetItem* )), this, SLOT(slotItemSelected(QListWidgetItem*)));
			modulesScroller->setWidget(modulesView);
			moduleTabs->addTab(modulesScroller, item.caption);
			overviewPages.append(modulesScroller);
		}
	}

	windowStack->setMinimumSize(700, 500);
	windowStack->addWidget(moduleTabs);
	windowStack->setCurrentWidget(moduleTabs);
	setCentralWidget(windowStack);
}

void MainWindow::buildActions()
{
	addAction(actionCollection()->addAction(KStandardAction::Quit, qobject_cast<QObject*>(this), SLOT(close())));

	resetModule = actionCollection() -> addAction("resetModule");
	resetModule->setText(i18n("Undo Changes"));
	connect(resetModule, SIGNAL(triggered()), this, SLOT(close()));
	resetModule->setEnabled(false);
	addAction(resetModule);

	defaultModule = actionCollection() -> addAction("defaultModule");
	defaultModule->setText(i18n("Reset to Defaults"));
	connect(defaultModule, SIGNAL(triggered()), this, SLOT(showAllModules()));;
	defaultModule->setEnabled(false);
	addAction(defaultModule);

	showAllAction = actionCollection()->addAction("showAll");
	showAllAction->setIcon( KIcon(QApplication::layoutDirection() == Qt::RightToLeft?"go-next":"go-previous") );
	showAllAction->setText( i18n("Overview") );
	connect(showAllAction, SIGNAL(triggered()), this, SLOT(showAllModules()));
	showAllAction->setEnabled(false);
	showAllAction->setShortcut(i18n("Ctrl+O"));
	addAction(showAllAction);

	aboutModuleAction = actionCollection() -> addAction("help_about_module");
	aboutModuleAction->setText(i18n("About Current Module"));
	connect(aboutModuleAction, SIGNAL(triggered()), this, SLOT(aboutCurrentModule()));
	addAction(aboutModuleAction);

	resetModuleHelp();

	QWidget *searchWid = new QWidget( this );
	QLabel * searchIcon = new QLabel( searchWid );
	searchIcon->setPixmap( BarIcon( "system-search" ) );
	QLabel *searchLabel = new QLabel( searchWid );
	searchLabel->setObjectName( QLatin1String("SearchLabel"));
	searchLabel->setText( i18n("S&earch:") );
	searchLabel->setFont(KGlobalSettings::toolBarFont());
	searchLabel->setMargin(2);
	QHBoxLayout * hlay = new QHBoxLayout( searchWid );
	hlay->addWidget( searchIcon );
	hlay->addWidget( searchLabel );
	searchWid->setLayout( hlay );

	searchText = new KAction( this );
	searchText->setDefaultWidget(searchWid);

	//actionCollection()->addAction( "spacer", new KToolBarSpacerAction( this ) );
	actionCollection()->addAction( "searchText", searchText );
	searchText->setShortcut(Qt::Key_F6);
	addAction(searchText);

	// Search edit box and result labels
	QWidget *hbox = new QWidget(0);

	KcmSearch* search = new KcmSearch(&modulesViewList, hbox);
	search->setObjectName(QLatin1String("search"));
	connect(search, SIGNAL(searchHits(const QString &, int *, int)), this, SLOT(slotSearchHits(const QString &, int *, int)));
	searchLabel->setBuddy( search );
	connect(searchText, SIGNAL(triggered()), search, SLOT(setFocus()));

	QWidget* vbox = new QWidget(hbox);
	generalHitLabel = new QLabel(vbox);
	advancedHitLabel = new QLabel(vbox);

	QVBoxLayout* vlayout = new QVBoxLayout;
	vlayout->addWidget(generalHitLabel);
	vlayout->addWidget(advancedHitLabel);
	vlayout->setStretchFactor(generalHitLabel,1);
	vlayout->setStretchFactor(advancedHitLabel,1);
	vbox->setLayout(vlayout);

	QHBoxLayout* hlayout = new QHBoxLayout;
	hlayout->addWidget(search);
	hlayout->addWidget(vbox);
	hlayout->setStretchFactor(search,1);
	hlayout->setStretchFactor(vbox,1);
	hbox->setLayout(hlayout);

	searchAction = new KAction( "none", this );
	searchAction->setDefaultWidget(hbox);
	actionCollection()->addAction( "search", searchAction );
	searchAction->setShortcutConfigurable( false );
	hbox->setWhatsThis( i18n("Search Bar<p>Enter a search term.</p>") );

	// Now it's time to draw our display
	foreach( const MenuItem &item , menu->menuList() ) {
		if( item.menu ) {
			KServiceGroup::Ptr group = KServiceGroup::group( item.subMenu );
			if ( !group ){
				kDebug() << "Invalid Group \"" << item.subMenu << "\".  Check your installation.";
				continue;
			}

			KToggleAction *newAction = new KToggleAction( KIcon(group->icon()), group->caption(), this);
			connect(newAction, SIGNAL(toggled(bool)), this, SLOT(slotTopPage()));

			pageActions.append(newAction);
			kDebug() << "relpath is :" << group->relPath();
		}
	}
}

void MainWindow::aboutCurrentModule()
{
	if(!groupWidget) {
		return;
	}

	KCModuleProxy* module = groupWidget->currentModule();
	if( module && module->aboutData() ){
		KAboutApplicationDialog dlg( module->aboutData() );
		dlg.exec();
	}
}

void MainWindow::groupModulesFinished()
{
	showAllModules();
}

void MainWindow::showAllModules()
{
	windowStack->setCurrentWidget(moduleTabs);

	// Reset the widget for normal all widget viewing
	groupWidget = 0;
	widgetChange();

	showAllAction->setEnabled(false);
	aboutModuleAction->setEnabled(false);

	searchText->setEnabled(true);
	searchAction->setEnabled(true);

	KToggleAction *currentRadioAction;
	foreach ( currentRadioAction, pageActions ) {
		currentRadioAction->setEnabled(true);
	}

	resetModuleHelp();
}

void MainWindow::slotItemSelected( QListWidgetItem *item ){
	ModuleIconItem *mItem = (ModuleIconItem *)item;

	if( !mItem )
		return;
	// Because some KCMultiWidgets take an age to load, it is possible
	// for the user to click another ModuleIconItem while loading.
	// This causes execution to return here while the first groupWidget is shown
	if ( groupWidget )
		return;

	kDebug() << "item selected: " << item->text();
	groupWidget = moduleItemToWidgetDict[mItem];
	scrollView = moduleItemToScrollerDict[mItem];

	if(groupWidget==0) {
		QList<KCModuleInfo> list = mItem->modules;

		scrollView = new QScrollArea(windowStack);
		groupWidget = new KCMultiWidget(0, scrollView, Qt::NonModal); // THAT ZERO IS NEW (actually the 0 can go, jr)
		scrollView->setWidget(groupWidget);
		scrollView->setWidgetResizable(true);
		windowStack->addWidget(scrollView);
		moduleItemToScrollerDict.insert(mItem,scrollView);
		moduleItemToWidgetDict.insert(mItem,groupWidget);

		connect(groupWidget, SIGNAL(aboutToShow( KCModuleProxy * )), this, SLOT(updateModuleHelp( KCModuleProxy * )));
		connect(groupWidget, SIGNAL(finished()), this, SLOT(groupModulesFinished()));
		connect(groupWidget, SIGNAL(close()), this, SLOT(showAllModules()));

		QList<KCModuleInfo>::const_iterator it;
		for ( it = list.begin(); it != list.end(); ++it ){
			qDebug("adding %s %s", qPrintable((*it).moduleName()), qPrintable((*it).fileName()));
			groupWidget->addModule(	*it );
		}
	}

	// calling this with a shown KCMultiWidget sets groupWidget to 0
	// which makes the show() call below segfault.  The groupWidget test
	// above should prevent execution reaching here while the KCMultiWidget is
	// visible
	windowStack->setCurrentWidget( scrollView );

	setCaption( mItem->text() );
	showAllAction->setEnabled(true);
	//searchText->setEnabled(false);
	//searchClear->setEnabled(false);
	//searchAction->setEnabled(false);

	KToggleAction *currentRadioAction;
	foreach ( currentRadioAction, pageActions ) {
		currentRadioAction->setEnabled(false);
	}

	groupWidget->show();

	// We resize and expand the window if necessary, but only once the window has been updated.
	// Some modules seem to dynamically change thier size. The new size is only available
	// once the dialog is updated. :-/ -SBE

	//disable resizing, goes against HIG
	//http://wiki.openusability.org/guidelines/index.php/Checklist_Configuration_Dialogs - jriddell
	//QTimer::singleShot(0,this,SLOT(timerResize()));
}

//this method not called, see above
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
		aboutModuleAction->setText(i18nc("Help menu->about <modulename>", "About %1",
				                             currentModule->moduleInfo().moduleName().replace("&","&&")));
		aboutModuleAction->setIcon(KIcon(currentModule->moduleInfo().icon()));
		aboutModuleAction->setEnabled(true);
	}
	else {
		resetModuleHelp();
	}
}

void MainWindow::resetModuleHelp() {
	aboutModuleAction->setText(i18n("About Current Module"));
	aboutModuleAction->setIcon(QIcon());
	aboutModuleAction->setEnabled(false);
}

void MainWindow::widgetChange() {
	QString name;
	if( groupWidget && groupWidget->currentModule()) {
		name = groupWidget->currentModule()->moduleInfo().moduleName();
	}

	if( !groupWidget ) {
		setCaption(QString());

		ModulesView *modulesView;
		foreach( modulesView, modulesViewList ) {
			modulesView->clearSelection();
		}
	}
}

void MainWindow::slotTopPage() {
	KToggleAction *clickedRadioAction = (KToggleAction *)sender();
	selectedPage = pageActions.indexOf(clickedRadioAction);

	KToggleAction *currentRadioAction;
	foreach ( currentRadioAction, pageActions ) {
		currentRadioAction->setChecked(currentRadioAction==clickedRadioAction);
	}

	windowStack->setCurrentWidget(overviewPages.at(selectedPage));
}

void MainWindow::slotSearchHits(const QString &query, int *hitList, int length) {
	if(query.isEmpty()) {
		generalHitLabel->setText(QString());
		advancedHitLabel->setText(QString());
	} else {

		if(length>=1) {
			generalHitLabel->setText(i18np("%1 hit in General","%1 hits in General",hitList[0]));
		}

		if(length>=2) {
			advancedHitLabel->setText(i18np("%1 hit in Advanced","%1 hits in Advanced",hitList[1]));
		}

	}
}

#include "mainwindow.moc"
