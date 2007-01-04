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
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef MODULESVIEW_H
#define MODULESVIEW_H

#include <kiconview.h>

/**
 * Overloaded to give a larger default size that fits with text of two lines.
 */
class RowIconView : public KIconView
{

public:
	RowIconView( QWidget* parent, const char *name=0 )
					: KIconView( parent, name ){ };
		
	// Figure out the hight/width to have only one row
	QSize minimumSizeHint() const {
		int width = 0;
		/*
		for ( QIconViewItem *item = firstItem(); item; item = item->nextItem() )
			width += item->width();
		width += spacing()*(count())+(margin()+frameWidth()+lineWidth()+midLineWidth())*2 ;
		*/

		width = count()*gridX()+frameWidth()*2;
		
		int height = 0;
		for ( QIconViewItem *item = firstItem(); item; item = item->nextItem() )
			if(item->height() > height)
				height = item->height();
		// I honestly don't know where the 4+4 is coming from...
		// What other spacing did I miss?
		height += (margin()+frameWidth()+spacing()+lineWidth()+midLineWidth())*2+8;
	
/*	
		int h = fontMetrics().height();
		if ( h < 10 )
			h = 10;
		int f = 2 * frameWidth();
		int height = ( 2*h ) + f + spacing() * 2 + 32 + lineWidth()*2 + 10;
	*/	
		return QSize( width, height );
	};

};

class QBoxLayout;
class KCModuleMenu;

/**
 * This widget contains the IconView's of all of the modules etc
 * It is the basic thing that users see.
 */
class ModulesView : public QWidget
{
	// To search the groups
	friend class KcmSearch;

Q_OBJECT
public:
	void clearSelection();
 QString displayName; 

signals:
	void itemSelected( QIconViewItem* item );

public:
	ModulesView( KCModuleMenu *rootMenu, const QString &menuPath, QWidget *parent=0, const char *name=0 );
	~ModulesView();

private:
	QValueList<RowIconView*> groups;
	KCModuleMenu *rootMenu;
	QString menuPath;

	void createRow( const QString &parentPath, QBoxLayout *layout );
};

#endif // MODULESVIEW_H

