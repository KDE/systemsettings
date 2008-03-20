
ADD_SUBDIRECTORY( categories ) 

########### next target ###############

SET(systemsettings_SRCS
   kcmodulemodel.cpp
   kcmultiwidget.cpp
   main.cpp
   mainwindow.cpp
   menuitem.cpp
   moduleiconitem.cpp
   moduletab.cpp
   moduletabbar.cpp
   )


KDE4_ADD_EXECUTABLE(systemsettings ${systemsettings_SRCS})

# Uses KIO for KRun and KUtils for KCModuleInfo
TARGET_LINK_LIBRARIES(systemsettings ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS} )

INSTALL( TARGETS systemsettings DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES systemsettings.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES systemsettingsrc DESTINATION ${SYSCONF_INSTALL_DIR} )
install( FILES systemsettingsui.rc DESTINATION ${DATA_INSTALL_DIR}/systemsettings )


