/**
 * This file is part of the System Preferences package
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "kcmsearch.h"

#include <qregexp.h>
#include <kdebug.h>

#include "modulesview.h"
#include "moduleiconitem.h"

KcmSearch::KcmSearch( ModulesView *mainView, QWidget *parent, const char *name )
				: KIconViewSearchLine(parent, mainView->groups[0], name){
	this->mainView = mainView;
};

void KcmSearch::updateSearch( const QString &search ) {
	QValueList<RowIconView*>::iterator it;
	for ( it = mainView->groups.begin(); it != mainView->groups.end(); ++it ){
		QIconViewItem *item = (*it)->firstItem();
		while( item ) {
			((ModuleIconItem*)item)->loadIcon(itemMatches(item, search));
			item = item->nextItem();
		}
	}
}

bool KcmSearch::itemMatches( const KCModuleInfo &module, const QString &search ) const
{
	// Look in keywords
	QStringList kw = module.keywords();
	for(QStringList::ConstIterator it = kw.begin(); it != kw.end(); ++it) {
		QString name = (*it).lower();
		if ( QRegExp(search+"*", false, true).search(name) >= 0){
			//kdDebug() << "MATCH:" << module.moduleName().latin1()
			//				  << "keyword:" <<  name.latin1() << endl;
			return true;
		}
	}
	
	// Don't forget to check the name :) 
	if ( QRegExp(search+"*", false, true).search(module.moduleName()) >= 0)
		return true;
	
	//kdDebug() << "No MATCH:" << module.moduleName().latin1() << endl;
	return false;
}

bool KcmSearch::itemMatches( const QIconViewItem *item, const QString & search ) const
{
	if( !item )
		return false;

	ModuleIconItem *mItem = (ModuleIconItem*)item;
	QValueList<KCModuleInfo>::iterator it;
	for ( it = mItem->modules.begin(); it != mItem->modules.end(); ++it ){
		if( itemMatches( (*it), search ) )
			return true;
	}
	return false;
}


#include "kcmsearch.moc"
