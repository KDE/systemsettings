PROJECT(systemsettings)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

FIND_PACKAGE(KDE4 REQUIRED)


ADD_DEFINITIONS (-DQT3_SUPPORT -DKDE3_SUPPORT)
ADD_DEFINITIONS (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)

SET(CMAKE_VERBOSE_MAKEFILE ON)

ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

LINK_DIRECTORIES(${KDE4_LIB_DIR})

ADD_SUBDIRECTORY( categories ) 

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

kde4_install_icons( ${ICON_INSTALL_DIR}  )


