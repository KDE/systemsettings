set(sidebar_mode_SRCS
    SidebarMode.cpp
    SidebarMode.h
)

add_library(systemsettings_sidebar_mode MODULE ${sidebar_mode_SRCS})

target_link_libraries(systemsettings_sidebar_mode systemsettingsview
    KF6::ItemViews
    KF6::ItemModels
    KF6::KCMUtils
    KF6::I18n
    KF6::KIOWidgets
    KF6::Service
    KF6::XmlGui
    KF6::Package
    Qt::Qml
    Qt::Quick
    Qt::QuickWidgets
)

install(TARGETS systemsettings_sidebar_mode DESTINATION ${KDE_INSTALL_PLUGINDIR}/systemsettingsview)
install(DIRECTORY package/ DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kpackage/genericqml/org.kde.systemsettings.sidebar)
