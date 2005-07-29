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
#  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.
# 

AM_CPPFLAGS= $(all_includes)
METASOURCES = AUTO

SUBDIRS = . menu

bin_PROGRAMS = systemsettings

systemsettings_SOURCES = kcmodulemenu.cpp kcmsearch.cpp kcmultiwidget.cpp main.cpp mainwindow.cpp moduleiconitem.cpp modulesview.cpp

EXTRA_DIST = kcmodulemenu.h kcmultiwidget.h moduleiconitem.h version.h kcmsearch.h mainwindow.h modulesview.h

systemsettings_LDADD  = $(LIB_KDEUI) $(LIB_KUTILS)
systemsettings_LDFLAGS = $(all_libraries) -module -avoid-version

appdata_DATA = systemsettingsui.rc
appdatadir = $(kde_datadir)/systemsettings

xdg_apps_DATA = systemsettings.desktop

KDE_ICON=AUTO

# KDE4 will have this by default 
conf_DATA = systemsettingsrc
confdir = $(kde_confdir)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcontrol.pot
