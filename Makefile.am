#
#  Copyright (c) 2005 Benjamin Meyer <ben+systempreferences@meyerhome.net>
# 
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
# 

AM_CPPFLAGS= $(all_includes)
METASOURCES = AUTO

SUBDIRS = . menu

bin_PROGRAMS = systempreferences

systempreferences_SOURCES = kcmodulemenu.cpp kcmsearch.cpp kcmultiwidget.cpp main.cpp mainwindow.cpp moduleiconitem.cpp modulesview.cpp

EXTRA_DIST = kcmodulemenu.h kcmultiwidget.h moduleiconitem.h version.h kcmsearch.h mainwindow.h modulesview.h

systempreferences_LDADD  = $(LIB_KDEUI) $(LIB_KUTILS)
systempreferences_LDFLAGS = $(all_libraries) -module -avoid-version

appdata_DATA = systempreferencesui.rc
appdatadir = $(kde_datadir)/systempreferences

xdg_apps_DATA = systempreferences.desktop

KDE_ICON=AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcontrol.pot
