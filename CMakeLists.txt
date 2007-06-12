PROJECT(systemsettings)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

FIND_PACKAGE(KDE4 REQUIRED)


ADD_DEFINITIONS (-DQT3_SUPPORT -DKDE3_SUPPORT)
ADD_DEFINITIONS (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)


FIND_PACKAGE(KDE4 REQUIRED)

SET(CMAKE_VERBOSE_MAKEFILE ON)

ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

LINK_DIRECTORIES(${KDE4_LIB_DIR})

ADD_SUBDIRECTORY( menu ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )
CONFIGURE_FILE (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )


########### next target ###############

SET(systemsettings_SRCS
   kcmodulemenu.cpp
   kcmsearch.cpp
   kcmultiwidget.cpp
   main.cpp
   mainwindow.cpp
   moduleiconitem.cpp
   modulesview.cpp
   kcscrollview.cpp
#   kicongrouppage.cpp
   )

KDE4_AUTOMOC(${systemsettings_SRCS})

KDE4_ADD_EXECUTABLE(systemsettings ${systemsettings_SRCS})

TARGET_LINK_LIBRARIES(systemsettings  ${QT_AND_KDECORE_LIBS} kdeui kutils ${KDE4_KDE3SUPPORT_LIBS} )

INSTALL( TARGETS systemsettings DESTINATION bin)


########### install files ###############

install( FILES systemsettings.desktop DESTINATION ${XDG_APPS_DIR} )
install( FILES systemsettingsrc DESTINATION ${SYSCONF_INSTALL_DIR} )
install( FILES systemsettingsui.rc DESTINATION ${DATA_INSTALL_DIR}/systemsettings )

KDE4_INSTALL_ICONS( hicolor )



#original Makefile.am contents follow:

##
##  Copyright (c) 2005 Benjamin Meyer <ben+systempreferences@meyerhome.net>
## 
##  This program is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.
## 
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
## 
##  You should have received a copy of the GNU General Public License
##  along with this program; if not, write to the Free Software
##  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.
## 
#
#AM_CPPFLAGS= $(all_includes)
#METASOURCES = AUTO
#
#SUBDIRS = . menu
#
#bin_PROGRAMS = systemsettings
#
#systemsettings_SOURCES = kcmodulemenu.cpp kcmsearch.cpp kcmultiwidget.cpp main.cpp mainwindow.cpp moduleiconitem.cpp modulesview.cpp kcscrollview.cpp
#
#EXTRA_DIST = kcmodulemenu.h kcmultiwidget.h moduleiconitem.h version.h kcmsearch.h mainwindow.h modulesview.h kcscrollview.h
#
#systemsettings_LDADD  = $(LIB_KDEUI) $(LIB_KUTILS)
#systemsettings_LDFLAGS = $(all_libraries) -module -avoid-version
#
#appdata_DATA = systemsettingsui.rc
#appdatadir = $(kde_datadir)/systemsettings
#
#xdg_apps_DATA = systemsettings.desktop
#
#KDE_ICON=AUTO
#
## KDE4 will have this by default 
#conf_DATA = systemsettingsrc
#confdir = $(kde_confdir)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/systemsettings.pot
