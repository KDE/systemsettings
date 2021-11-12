
include_directories( ToolTips )

set( systemsettings_SRCS
    main.cpp
    SettingsBase.cpp
    SettingsBase.h
    ToolTips/tooltipmanager.cpp
    ToolTips/tooltipmanager.h
     )

ecm_qt_declare_logging_category(systemsettings_SRCS HEADER systemsettings_app_debug.h IDENTIFIER SYSTEMSETTINGS_APP_LOG CATEGORY_NAME org.kde.systemsettings.app DESCRIPTION "systemsettings" EXPORT SYSTEMSETTINGS)

# port it it's in kdelibs4support
# kde4_add_app_icon( systemsettings_SRCS "${KDE4_ICON_INSTALL_DIR}/oxygen/*/categories/preferences-system.png" )
kconfig_add_kcfg_files( systemsettings_SRCS BaseConfig.kcfgc )

add_executable(systemsettings ${systemsettings_SRCS})
target_compile_definitions(systemsettings PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries( systemsettings systemsettingsview
    KF5::Crash
    KF5::ItemViews
    KF5::KCMUtils
    KF5::I18n
    KF5::IconThemes
    KF5::KIOWidgets
    KF5::Service
    KF5::WindowSystem
    KF5::XmlGui
    KF5::DBusAddons
    KF5::ConfigGui
    KF5::QuickAddons
    KF5::GuiAddons # UrlHanlder handles help:/ urls
    PW::KWorkspace
)


add_custom_command(TARGET systemsettings POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink systemsettings systemsettings5)

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/systemsettings5 DESTINATION ${KDE_INSTALL_FULL_BINDIR}/ )

install( TARGETS systemsettings ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES systemsettings.kcfg DESTINATION ${KDE_INSTALL_DATADIR}/systemsettings )
install( FILES systemsettingsui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/systemsettings )
install( PROGRAMS kdesystemsettings.desktop systemsettings.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
install( DIRECTORY DESTINATION "${KDE_INSTALL_FULL_DATAROOTDIR}/kglobalaccel" )
install(
    CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink \"${KDE_INSTALL_FULL_APPDIR}/systemsettings.desktop\" \"\$ENV{DESTDIR}${KDE_INSTALL_FULL_DATAROOTDIR}/kglobalaccel/systemsettings.desktop\")"
)
install(FILES org.kde.systemsettings.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
