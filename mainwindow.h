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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <kmainwindow.h>
#include <kcmoduleinfo.h>

class QWidgetStack;
class QIconViewItem;
class ModulesView;
class KAction;
class KWidgetAction;
class KCModule;
class KCModuleProxy;
class KcmSearch;
class QLabel;

class MainWindow : public KMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent=0, const char *name=0);
	~MainWindow();

private slots:
	void slotItemSelected( QIconViewItem* item );
	void slotRootItemSelected( QIconViewItem* item );
	void showAllModules();
	void reportBug();
	void slotClearSearch();
	void slotSearchAll( const QString &text );

private:
	QWidgetStack *windowStack;
	QMap<QWidget*,KWidgetAction*> searchActions;
	QMap<QWidget*,KcmSearch*> searchers;
	ModulesView *rootView;

	KAction *resetModule;
	KAction *defaultModule;

	KAction *showAllAction;
	KWidgetAction *searchText;
	KAction *searchClear;
	KAction *reportBugAction;

	QLabel *searchLabel;

	void buildMainWidget();
	void buildActions();
};

#endif

