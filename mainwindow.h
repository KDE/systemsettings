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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt
#include <QtCore/QHash>

// KDE
#include <kxmlguiwindow.h>

class QStackedWidget;
class KCMultiWidget;
class ModulesView;
class KAction;
class KToolBarLabelAction;
class KCModule;
class KCModuleProxy;
class QListWidgetItem;
class QScrollArea;
class KTabWidget;
class QLabel;
class KToggleAction;
class KAboutData;
class QAction;
class KCModuleMenu;
class ModuleIconItem;

class MainWindow : public KXmlGuiWindow
{
	Q_OBJECT

public:
	explicit MainWindow(const QString &menufile="systemsettings", QWidget *parent=0);
	~MainWindow();
        virtual void closeEvent ( QCloseEvent * );

private slots:
	void slotItemSelected( QListWidgetItem* item );
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
	QStackedWidget *windowStack;
	KTabWidget *moduleTabs;

	QList<ModulesView*> modulesViewList;

	KCMultiWidget *groupWidget;
	QScrollArea *scrollView;

	QHash<ModuleIconItem*,KCMultiWidget*> moduleItemToWidgetDict;

	QList<KToggleAction*> pageActions;
	QList<QScrollArea*> overviewPages;
	int selectedPage;

	QAction *resetModule;
	QAction *defaultModule;

	QAction *showAllAction;
	//KToolBarLabelAction *searchText;
	KAction *searchText;
	KAction *searchClear;
	//KToolBarLabelAction *searchAction;
	KAction *searchAction;

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

