/**
 * This file is part of the System Settings package
 * Copyright (C) 2005 Benjamin C Meyer (ben+systempreferences at meyerhome dot net)
 *           (C) 2007 Will Stephenson <wstephenson@kde.org>
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

#include <kstandardaction.h>
#include <ktoggletoolbaraction.h>
#include <ktoolbarspaceraction.h>
#include <kaboutapplicationdialog.h>
#include <QLabel>
#include <QStackedWidget>
#include <klocale.h>
#include <qlayout.h>
#include <KGlobalSettings>
#include <kiconloader.h>
#include <kactioncollection.h>
#include <qapplication.h>
#include <kdebug.h>
#include <kcmoduleproxy.h>
#include <kmenubar.h>
#include <ktoggleaction.h>
#include <qscrollarea.h>
#include <kcmoduleinfo.h>
#include <ktabwidget.h>
#include <kservicetypetrader.h>
#include <kcategorizedsortfilterproxymodel.h>
#include <KLineEdit>
#include <KDialog> // for spacing

#include "kcategorizedview.h"
#include "kcategorydrawer.h"
#include "kcmodulemodel.h"
#include "kcmultiwidget.h"
#include "menuitem.h"
#include "moduleiconitem.h"


Q_DECLARE_METATYPE(MenuItem *)

MainWindow::MainWindow(QWidget *parent) :
    KXmlGuiWindow(parent), categories( KServiceTypeTrader::self()->query("SystemSettingsCategory") ),
    modules( KServiceTypeTrader::self()->query("KCModule") ),
    rootItem(new MenuItem( true, 0 )),
    groupWidget(NULL), selectedPage(0) {

	// Load the menu structure in from disk.
    readMenu( rootItem );
    qStableSort( rootItem->children.begin(), rootItem->children.end(), pageLessThan ); // sort tabs by weight
	moduleTabs = new KTabWidget(this, QTabWidget::North|QTabWidget::Rounded);
	buildActions();
	buildMainWidget();
	setupGUI(ToolBar|Save|Create,QString());
	//widgetChange();
	menuBar()->hide();
}

MainWindow::~MainWindow()
{
	delete moduleTabs;
    delete rootItem;
}

void MainWindow::readMenu( MenuItem * parent )
{
    // look for any categories inside this level, and recurse into them
    int depth = 0;
    MenuItem * current = parent;
    while ( current && current->parent ) {
        depth++;
        current = current->parent;
    }

    QString space;
    space.fill( ' ', depth * 2 );
    kDebug() << space << "Looking for children in '" << parent->name << "'";
    for (int i = 0; i < categories.size(); ++i) {
        KService::Ptr entry = categories.at(i);
        QString parentCategory = entry->property("X-KDE-System-Settings-Parent-Category").toString();
        QString category = entry->property("X-KDE-System-Settings-Category").toString();
        //kDebug() << "Examining category " << parentCategory << "/" << category;
        if ( parentCategory == parent->name ) {
            kDebug() << space << "found category '" << entry->name() << "' " << entry->desktopEntryPath();
            KCModuleInfo module(entry->entryPath());

            MenuItem * menuItem = new MenuItem(true, parent);
            menuItem->name = category;
            menuItem->service = entry;
            menuItem->item = module;
            readMenu( menuItem );
        }
    }

    // scan for any modules at this level and add them
    for (int i = 0; i < modules.size(); ++i) {
        KService::Ptr entry = modules.at(i);
        QString category = entry->property("X-KDE-System-Settings-Parent-Category").toString();
        //kDebug() << "Examining module " << category;
        if(!parent->name.isEmpty() && category == parent->name ) {
            kDebug() << space << "found module '" << entry->name() << "' " << entry->desktopEntryPath();
            // Add the module info to the menu
            KCModuleInfo module(entry->entryPath());
            kDebug() << space << "filename is " << module.fileName();
            //append(module);
            MenuItem * infoItem = new MenuItem(false, parent);
            infoItem->name = category;
            infoItem->service = entry;
            infoItem->item = module;
        }
    }
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
    moduleTabs->show();

    foreach ( MenuItem* item, rootItem->children ) {
        model = new KCModuleModel( item, this );
        KCategoryDrawer * drawer = new KCategoryDrawer;
        KCategorizedView * tv = new KCategorizedView( this );
        tv->setSelectionMode(QAbstractItemView::SingleSelection);
        tv->setSpacing(KDialog::spacingHint());
        tv->setCategoryDrawer( drawer );
        tv->setViewMode( QListView::IconMode );
        tv->setItemDelegate( new ModuleIconItemDelegate( this ) );
        tv->setMouseTracking( true );
        tv->viewport()->setAttribute( Qt::WA_Hover );
        KCategorizedSortFilterProxyModel * kcsfpm = new KCategorizedSortFilterProxyModel( this );
        kcsfpm->setCategorizedModel( true );
        kcsfpm->setSourceModel( model );
        kcsfpm->setFilterRole( KCModuleModel::UserFilterRole );
        kcsfpm->setFilterCaseSensitivity( Qt::CaseInsensitive );
        kcsfpm->sort( 0 );
        tv->setModel( kcsfpm );
        connect( tv,
                SIGNAL(activated(const QModelIndex&)),
                SLOT(selectionChanged(const QModelIndex&)) );
        if (KGlobalSettings::singleClick()) {
            connect( tv, SIGNAL(clicked(const QModelIndex&)),
                     SLOT(selectionChanged(const QModelIndex&)));
        } else {
            connect( tv, SIGNAL(doubleClicked(const QModelIndex&)),
                     SLOT(selectionChanged(const QModelIndex&)));
        }
        connect( search, SIGNAL(textChanged(const QString&)),
                kcsfpm, SLOT(setFilterRegExp(const QString&)));
        connect( kcsfpm, SIGNAL(layoutChanged()),
                SLOT(updateSearchHits()) );
        moduleTabs->addTab(tv, item->service->name() );
        // record the index of the newly added tab so that we can later update the label showing
        // number of search hits
        modelToTabHash.insert( kcsfpm, moduleTabs->count() - 1 );
    }
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
	QWidget *hbox = new QWidget( this );

    search = new KLineEdit( hbox );
	search->setObjectName(QLatin1String("search"));
    search->setClearButtonShown( true );
    search->setFocusPolicy( Qt::StrongFocus );
	searchLabel->setBuddy( search );
	connect(searchText, SIGNAL(triggered()), search, SLOT(setFocus()));

	QWidget* vbox = new QWidget(hbox);
	// Set a non empty content to prevent the toolbar from getting taller when
	// starting a search (at least with Oxygen style).
	generalHitLabel = new QLabel(" ", vbox);
	advancedHitLabel = new QLabel(" ", vbox);

	QVBoxLayout* vlayout = new QVBoxLayout;
	vlayout->setMargin(0);
	vlayout->setSpacing(0);
	vlayout->addWidget(generalHitLabel);
	vlayout->addWidget(advancedHitLabel);
	vlayout->setStretchFactor(generalHitLabel,1);
	vlayout->setStretchFactor(advancedHitLabel,1);
	vbox->setLayout(vlayout);

	QHBoxLayout* hlayout = new QHBoxLayout;
	hlayout->setMargin(0);
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
#if 0
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
#endif
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
    search->setEnabled(true);
	searchAction->setEnabled(true);

	KToggleAction *currentRadioAction;
	foreach ( currentRadioAction, pageActions ) {
		currentRadioAction->setEnabled(true);
	}

	resetModuleHelp();
}

void MainWindow::selectionChanged( const QModelIndex& selected )
{
    if ( !selected.isValid() )
        return;

    if ( selected.isValid() ) {
        MenuItem * mItem = selected.data( Qt::UserRole ).value<MenuItem*>();
        if ( mItem ) {
            kDebug() << "Selected item: " << mItem->service->name();
            kDebug() << "Comment:       " << mItem->service->comment();
        } else {
            kDebug() << ":'( Got dud pointer from " << selected.data( Qt::DisplayRole ).toString();
        }
        // Because some KCMultiWidgets take an age to load, it is possible
        // for the user to click another ModuleIconItem while loading.
        // This causes execution to return here while the first groupWidget is shown
        if ( groupWidget )
            return;

        groupWidget = moduleItemToWidgetDict[mItem->service];

        if( !groupWidget ) {
            groupWidget = new KCMultiWidget(windowStack, Qt::NonModal);
            windowStack->addWidget(groupWidget);
            moduleItemToWidgetDict.insert(mItem->service,groupWidget);

            connect(groupWidget, SIGNAL(aboutToShow( KCModuleProxy * )), this, SLOT(updateModuleHelp( KCModuleProxy * )));
            connect(groupWidget, SIGNAL(finished()), this, SLOT(groupModulesFinished()));
            connect(groupWidget, SIGNAL(close()), this, SLOT(showAllModules()));

            if ( mItem->children.isEmpty() ) {
                groupWidget->addModule( mItem->item );
            } else {
                foreach ( MenuItem * i, mItem->children ) {
                    kDebug() << "adding " , i->item.fileName();
                    groupWidget->addModule( i->item );
                }
            }
        }

        // calling this with a shown KCMultiWidget sets groupWidget to 0
        // which makes the show() call below segfault.  The groupWidget test
        // above should prevent execution reaching here while the KCMultiWidget is
        // visible
        windowStack->setCurrentWidget( groupWidget );

        setCaption( mItem->service->name() );
        showAllAction->setEnabled(true);
        searchText->setEnabled(false);
        search->setEnabled(false);
        searchAction->setEnabled(false);

        KToggleAction *currentRadioAction;
        foreach ( currentRadioAction, pageActions ) {
            currentRadioAction->setEnabled(false);
        }
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

        KCategorizedView * currentView = qobject_cast<KCategorizedView *>( moduleTabs->currentWidget() );
        currentView->selectionModel()->clear();
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

void MainWindow::updateSearchHits()
{
    // if the search lineedit is empty, clear the search labels
    if ( search->text().isEmpty() ) {
        generalHitLabel->setText(QString());
        advancedHitLabel->setText(QString());
    } else { // otherwise update the tab for the sender()
        for ( int i = 0; i < moduleTabs->count(); i++ ) {
            const KCategorizedSortFilterProxyModel * kcsfpm = static_cast<KCategorizedSortFilterProxyModel*>( sender() );
            if (kcsfpm && modelToTabHash.contains( kcsfpm ) ) {
                switch ( modelToTabHash[ kcsfpm ] ) {
                    case 0:
                        generalHitLabel->setText(i18np("%1 hit in General","%1 hits in General", kcsfpm->rowCount()));
                        break;
                    case 1:
                        advancedHitLabel->setText(i18np("%1 hit in Advanced","%1 hits in Advanced",kcsfpm->rowCount()));
                        break;
                    default:
                        kDebug() << "Hits found in top level system settings other than General, Advanced, and the UI is hardcoded to only indicate hits in these tabs";
                }
            }
        }
    }
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

bool pageLessThan( MenuItem *page1, MenuItem *page2 )
{
    return page1->item.weight() < page2->item.weight();
}

#include "mainwindow.moc"
