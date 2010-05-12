SET(classic_mode_srcs
    ClassicMode.cpp
    CategoryList.cpp
)

KDE4_ADD_UI_FILES( classic_mode_srcs configClassic.ui )
KDE4_ADD_PLUGIN(classic_mode ${classic_mode_srcs})

TARGET_LINK_LIBRARIES(classic_mode ${KDE4_KCMUTILS_LIBS} ${KDE4_KHTML_LIBS} systemsettingsview )

INSTALL( TARGETS classic_mode DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( FILES settings-classic-view.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
INSTALL( FILES main.html systemsettings-classic.css DESTINATION ${DATA_INSTALL_DIR}/systemsettings/classic/ )

