set( sidebar_mode_srcs
     SidebarMode.cpp
     CategoryDrawer.cpp
     CategorizedView.cpp
) 

add_library(systemsettings_sidebar_mode MODULE ${sidebar_mode_srcs})

target_link_libraries(systemsettings_sidebar_mode systemsettingsview
    KF5::ItemViews
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOWidgets
    KF5::Service
)

install( TARGETS systemsettings_sidebar_mode DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES settings-sidebar-view.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

