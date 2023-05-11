
include_directories(ToolTips)

set(systemsettings_SRCS
    systemsettings.qrc
    main.cpp
    SettingsBase.cpp
    SettingsBase.h
    ToolTips/tooltipmanager.cpp
    ToolTips/tooltipmanager.h
    sidebar/SidebarMode.cpp
    sidebar/SidebarMode.h
    resources.qrc
    ${debug_SRCS}
)

add_executable(systemsettings ${systemsettings_SRCS})
target_compile_definitions(systemsettings PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(systemsettings
    systemsettings_debug
    systemsettingsview
    KF6::Crash
    KF6::ItemViews
    KF6::ItemModels
    KF6::KCMUtils
    KF6::I18n
    KF6::IconThemes
    KF6::KIOWidgets
    KF6::Service
    KF6::WindowSystem
    KF6::XmlGui
    KF6::DBusAddons
    KF6::ConfigGui
    KF6::GuiAddons # UrlHanlder handles help:/ urls
    Qt::QuickWidgets
    Qt::Qml
    Qt::Quick
    PW::KWorkspace
)

install(TARGETS systemsettings ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES systemsettings.kcfg DESTINATION ${KDE_INSTALL_DATADIR}/systemsettings)
install(PROGRAMS kdesystemsettings.desktop systemsettings.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(DIRECTORY DESTINATION "${KDE_INSTALL_FULL_DATAROOTDIR}/kglobalaccel")
install(
    CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink \"${KDE_INSTALL_FULL_APPDIR}/systemsettings.desktop\" \"\$ENV{DESTDIR}${KDE_INSTALL_FULL_DATAROOTDIR}/kglobalaccel/systemsettings.desktop\")"
)
install(FILES org.kde.systemsettings.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES systemsettings.zsh RENAME _systemsettings DESTINATION ${KDE_INSTALL_ZSHAUTOCOMPLETEDIR})
