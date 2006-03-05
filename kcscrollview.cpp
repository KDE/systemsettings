/*
   Copyright (c) 2006 Simon Edwards <simon@simonzone.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include "kcscrollview.h"

KCScrollView::KCScrollView( QWidget * parent, const char * name, WFlags f) : QScrollView(parent,name,f) {
	setResizePolicy(AutoOneFit);
	mainChild = 0;
}

QSize KCScrollView::sizeHint() const {
	QSize vphint = mainChild->sizeHint();
	vphint.setWidth(vphint.width()+2*frameWidth());
	vphint.setHeight(vphint.height()+2*frameWidth());
	return vphint;
}

void KCScrollView::addChild(QWidget *child, int x, int y) {
	mainChild = child;
	QScrollView::addChild(child,x,y);
}
