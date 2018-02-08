
include_directories( ToolTips )

set( systemsettings_SRCS
     SystemSettingsApp.cpp
     SettingsBase.cpp
     ToolTips/tooltipmanager.cpp
     main.cpp )

ecm_qt_declare_logging_category(systemsettings_SRCS HEADER systemsettings_app_debug.h IDENTIFIER SYSTEMSETTINGS_APP_LOG CATEGORY_NAME org.kde.systemsettings.app)

# port it it's in kdelibs4support
# kde4_add_app_icon( systemsettings_SRCS "${KDE4_ICON_INSTALL_DIR}/oxygen/*/categories/preferences-system.png" )
ki18n_wrap_ui( systemsettings_SRCS configDialog.ui )
kconfig_add_kcfg_files( systemsettings_SRCS BaseConfig.kcfgc )
qt5_add_dbus_adaptor( systemsettings_SRCS org.kde.systemsettings.xml SystemSettingsApp.h SystemSettingsApp  )

add_executable( systemsettings5 ${systemsettings_SRCS})
target_compile_definitions(systemsettings5 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries( systemsettings5 systemsettingsview
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
)

install( TARGETS systemsettings5 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES systemsettings.kcfg DESTINATION ${KDE_INSTALL_DATADIR}/systemsettings )
install( FILES systemsettingsui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/systemsettings )
install( PROGRAMS kdesystemsettings.desktop systemsettings.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
