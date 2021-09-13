set( iconsquick_mode_srcs
     IconsMode.cpp
     ToolTips/tooltipmanager.cpp
) 

add_library(systemsettings_iconsquick_mode MODULE ${iconsquick_mode_srcs})

kcoreaddons_desktop_to_json(systemsettings_iconsquick_mode settings-iconsquick-view.desktop)

target_link_libraries(systemsettings_iconsquick_mode systemsettingsview
    KF5::ItemViews
    KF5::ItemModels
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOWidgets
    KF5::Service
    KF5::XmlGui
    KF5::Package
    KF5::ActivitiesStats
    Qt::Qml
    Qt::Quick
    Qt::QuickWidgets
)

install( TARGETS systemsettings_iconsquick_mode DESTINATION ${KDE_INSTALL_PLUGINDIR}/systemsettingsview )
install(DIRECTORY package/ DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kpackage/genericqml/org.kde.systemsettings.iconsquick)

