set( sidebar_mode_srcs
     SidebarMode.cpp
     ToolTips/tooltipmanager.cpp
) 

add_library(systemsettings_sidebar_mode MODULE ${sidebar_mode_srcs})

kcoreaddons_desktop_to_json(systemsettings_sidebar_mode settings-sidebar-view.desktop)

target_link_libraries(systemsettings_sidebar_mode systemsettingsview
    KF5::ItemViews
    KF5::ItemModels
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOWidgets
    KF5::Service
    KF5::XmlGui
    KF5::Package
    KF5::Declarative
    KF5::ActivitiesStats
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickWidgets
)

install( TARGETS systemsettings_sidebar_mode DESTINATION ${KDE_INSTALL_PLUGINDIR}/systemsettingsview )
install(DIRECTORY package/ DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kpackage/genericqml/org.kde.systemsettings.sidebar)

