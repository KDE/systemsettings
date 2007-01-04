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

#include <kmainwindow.h>
#include <kcmoduleinfo.h>
#include <ktabwidget.h>
#include <qptrdict.h>
#include <qlabel.h>
#include <kactionclasses.h>
#include <kiconviewsearchline.h> 

#include "kcscrollview.h"
#include "kcmodulemenu.h"

class QWidgetStack;
class QIconViewItem;
class KCMultiWidget;
class ModulesView;
class KAction;
class KWidgetAction;
class KCModule;
class KCModuleProxy;

class MainWindow : public KMainWindow
{
	Q_OBJECT

public:
	MainWindow(bool embed=true, const QString &menufile="systemsettings", 
									QWidget *parent=0, const char *name=0);
	~MainWindow();

private slots:
	void slotItemSelected( QIconViewItem* item );
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
	QWidgetStack *windowStack;
	KTabWidget *moduleTabs;

	QPtrList<ModulesView> modulesViewList;
	QPtrList<QIconView> viewList;

	KCMultiWidget *groupWidget;
	KCScrollView *scrollView;

	QPtrDict<KCMultiWidget> moduleItemToWidgetDict;
	QPtrDict<KCScrollView> moduleItemToScrollerDict;

	QPtrList<KRadioAction> pageActions;
	QPtrList<KCScrollView> overviewPages;
	int selectedPage;

	KAction *resetModule;
	KAction *defaultModule;
	
	KAction *showAllAction;
	KWidgetAction *searchText;
	KAction *searchClear;
	KWidgetAction *searchAction;

	KAction *aboutModuleAction;
	
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

