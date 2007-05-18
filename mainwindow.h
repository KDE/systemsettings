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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <kxmlguiwindow.h>
#include <kcmoduleinfo.h>
#include <ktabwidget.h>
#include <q3ptrdict.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3PtrList>
#include <kaction.h>
#include <ktoggleaction.h>
#include <k3iconviewsearchline.h> 
#include <kaboutdata.h>
#include <kaboutapplicationdialog.h>
#include <QAction>

#include "kcscrollview.h"
#include "kcmodulemenu.h"

class Q3WidgetStack;
class Q3IconViewItem;
class KCMultiWidget;
class ModulesView;
class KAction;
class KWidgetAction;
class KCModule;
class KCModuleProxy;

class MainWindow : public KXmlGuiWindow
{
	Q_OBJECT

public:
	MainWindow(bool embed=true, const QString &menufile="systemsettings", 
									QWidget *parent=0);
	~MainWindow();

private slots:
	void slotItemSelected( Q3IconViewItem* item );
	void showAllModules();
	void aboutCurrentModule();
	void updateModuleHelp( KCModuleProxy * );
	
	void resetModuleHelp();
	void groupModulesFinished();

	void widgetChange();
	void timerResize();
	void slotTopPage();
	void slotSearchHits(const QString &query, int *hitList, int length);

private:
	KCModuleMenu *menu;
	bool embeddedWindows;
	Q3WidgetStack *windowStack;
	KTabWidget *moduleTabs;

	Q3PtrList<ModulesView> modulesViewList;
	Q3PtrList<Q3IconView> viewList;

	KCMultiWidget *groupWidget;
	KCScrollView *scrollView;

	Q3PtrDict<KCMultiWidget> moduleItemToWidgetDict;
	Q3PtrDict<KCScrollView> moduleItemToScrollerDict;

	Q3PtrList<KToggleAction> pageActions;
	Q3PtrList<KCScrollView> overviewPages;
	int selectedPage;

	QAction *resetModule;
	QAction *defaultModule;
	
	QAction *showAllAction;
	KWidgetAction *searchText;
	KAction *searchClear;
	KWidgetAction *searchAction;

	QAction *aboutModuleAction;
	
	void buildMainWidget();
	void buildActions();

	QLabel *generalHitLabel;
	QLabel *advancedHitLabel;

  /**
   * If someone wants to report a bug
   * against a module with no about data
   * we construct one for him
   **/
  KAboutData *dummyAbout;
};

#endif

