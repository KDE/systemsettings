SET( icon_mode_srcs
     IconMode.cpp
     CategoryDrawer.cpp
     CategorizedView.cpp
) 

KDE4_ADD_PLUGIN(icon_mode ${icon_mode_srcs})

TARGET_LINK_LIBRARIES(icon_mode ${KDE4_KIO_LIBS} ${KDE4_KCMUTILS_LIBS} systemsettingsview )

INSTALL( TARGETS icon_mode DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( FILES settings-icon-view.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

