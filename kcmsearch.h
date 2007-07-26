/**
 * This file is part of the System Settings package
 * Copyright (C) 2005 Benjamin C Meyer
 *                    <ben+systempreferences at meyerhome dot net>
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

#ifndef KCMSEARCH_H
#define KCMSEARCH_H

#include <QList>
#include <QListWidget>
#include <klistwidgetsearchline.h>

class ModulesView;
class QListWidgetItem;
class KCModuleInfo;

/**
 * Searches all the ModuleIconItem's in MainWindow and "disables" the ones
 * whos keywords don't match the current search.
 */
class KcmSearch : public KListWidgetSearchLine
{
	Q_OBJECT

public:
	explicit KcmSearch( QList<ModulesView*> *moduleViewList, QWidget *parent = 0 );
	virtual ~KcmSearch();

public slots:
	/**
	 * Go through all of the iconView groups in mainView and update
	 */
	virtual void updateSearch( const QString &search = QString() );
	/**
	 * Check module associated with item or if a group check all modules of that group.
	 * @return true if search is in the module(s) keywords
	 */
	virtual bool itemMatches ( const QListWidgetItem* item, const QString& search ) const;

signals:
	void searchHits(const QString &query, int *hitList, int length);

private:
	/**
	 * Determine if module matches the search 
	 * @return true if search is in module's keywords
	 */
	bool itemMatches ( const KCModuleInfo &module, const QString &search ) const;

	// Friend class whos groups parsed, 
	QList<ModulesView*> *moduleViewList;

	// dummy listview to enable the search line
	QListWidget dummyListWidget;
};

#endif // KCMSEARCH_H

