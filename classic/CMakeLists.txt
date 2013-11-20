set(classic_mode_srcs
    ClassicMode.cpp
    CategoryList.cpp
)

kde4_add_ui_files( classic_mode_srcs configClassic.ui )
add_library(classic_mode MODULE ${classic_mode_srcs})

target_link_libraries(classic_mode systemsettingsview
    KF5::ItemViews
    KF5::KConfigCore
    KF5::KCMUtils
    KF5::KIconThemes
    KF5::KI18n
    KF5::KIOWidgets
    KF5::KService
    KF5::KHtml
    KF5::KDE4Support
)

install( TARGETS classic_mode DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES settings-classic-view.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES main.html systemsettings-classic.css DESTINATION ${DATA_INSTALL_DIR}/systemsettings/classic/ )

