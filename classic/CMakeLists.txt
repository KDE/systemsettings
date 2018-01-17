set(classic_mode_srcs
    ClassicMode.cpp
    CategoryList.cpp
)

ecm_qt_declare_logging_category(classic_mode_srcs HEADER systemsettings_classic_debug.h IDENTIFIER SYSTEMSETTINGS_CLASSIC_LOG CATEGORY_NAME org.kde.systemsettings.classic)

ki18n_wrap_ui( classic_mode_srcs configClassic.ui )
add_library(classic_mode MODULE ${classic_mode_srcs})

target_link_libraries(classic_mode systemsettingsview
    KF5::ItemViews
    KF5::ConfigCore
    KF5::KCMUtils
    KF5::IconThemes
    KF5::I18n
    KF5::KIOWidgets
    KF5::Service
    KF5::KHtml
)

install( TARGETS classic_mode DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES settings-classic-view.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES main.html systemsettings-classic.css DESTINATION ${DATA_INSTALL_DIR}/systemsettings/classic/ )

