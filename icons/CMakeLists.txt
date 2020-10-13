set( icon_mode_srcs
     IconMode.cpp
     CategoryDrawer.cpp
     CategorizedView.cpp
) 

add_library(icon_mode MODULE ${icon_mode_srcs})

kcoreaddons_desktop_to_json(icon_mode settings-icon-view.desktop)

target_link_libraries(icon_mode systemsettingsview
    KF5::ItemViews
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOWidgets
    KF5::Service
)

install( TARGETS icon_mode DESTINATION ${KDE_INSTALL_PLUGINDIR}/systemsettingsview )
