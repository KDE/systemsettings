SET( systemsettings_SRCS
     SystemSettingsApp.cpp
     SettingsBase.cpp
     main.cpp )

KDE4_ADD_UI_FILES( systemsettings_SRCS configDialog.ui )
QT4_ADD_DBUS_ADAPTOR( systemsettings_SRCS org.kde.systemsettings.xml SystemSettingsApp.h SystemSettingsApp  )

KDE4_ADD_EXECUTABLE( systemsettings ${systemsettings_SRCS})

TARGET_LINK_LIBRARIES( systemsettings ${KDE4_KIO_LIBS} systemsettingsview )

INSTALL( TARGETS systemsettings ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( FILES systemsettingsui.rc DESTINATION ${DATA_INSTALL_DIR}/systemsettings )
INSTALL( FILES systemsettings.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
