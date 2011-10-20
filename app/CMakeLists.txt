include_directories( ToolTips )

SET( systemsettings_SRCS
     SystemSettingsApp.cpp
     SettingsBase.cpp
     ToolTips/ktooltip.cpp
     ToolTips/ktooltipwindow.cpp
     ToolTips/tooltipmanager.cpp
     main.cpp )

KDE4_ADD_APP_ICON( systemsettings_SRCS "${KDE4_ICON_INSTALL_DIR}/oxygen/*/categories/preferences-system.png" )
KDE4_ADD_UI_FILES( systemsettings_SRCS configDialog.ui )
KDE4_ADD_KCFG_FILES( systemsettings_SRCS BaseConfig.kcfgc )
QT4_ADD_DBUS_ADAPTOR( systemsettings_SRCS org.kde.systemsettings.xml SystemSettingsApp.h SystemSettingsApp  )

KDE4_ADD_EXECUTABLE( systemsettings ${systemsettings_SRCS})

TARGET_LINK_LIBRARIES( systemsettings ${KDE4_KIO_LIBS} ${KDE4_KCMUTILS_LIBS} systemsettingsview )

INSTALL( TARGETS systemsettings ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( FILES systemsettingsui.rc systemsettings.kcfg DESTINATION ${DATA_INSTALL_DIR}/systemsettings )
INSTALL( PROGRAMS kdesystemsettings.desktop systemsettings.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
