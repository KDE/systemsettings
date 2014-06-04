
set(quick_mode_srcs
    Category.cpp
    host.cpp
    kuserproxy.cpp
    QuickMode.cpp
)

kde4_add_ui_files( quick_mode_srcs configClassic.ui )
add_library(quick_mode MODULE ${quick_mode_srcs})

target_link_libraries(quick_mode systemsettingsview
    Qt5::Quick
    Qt5::QuickWidgets
    KF5::ConfigCore
    KF5::I18n
    KF5::Service
    KF5::Plasma
)

install( TARGETS quick_mode DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES settings-quick-view.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

plasma_install_package(breeze org.kde.systemsettings.breeze packages package)
