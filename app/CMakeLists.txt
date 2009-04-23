SET( kcontrol4_SRCS
     KControlApp.cpp
     SettingsBase.cpp
     main.cpp )

KDE4_ADD_UI_FILES( kcontrol4_SRCS configDialog.ui )
QT4_ADD_DBUS_ADAPTOR( kcontrol4_SRCS org.kde.kcontrol4.xml KControlApp.h KControlApp  )

KDE4_ADD_EXECUTABLE( kcontrol4 ${kcontrol4_SRCS})

TARGET_LINK_LIBRARIES( kcontrol4 ${KDE4_KIO_LIBS} kcontrolview )

INSTALL( TARGETS kcontrol4 ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( FILES kcontrol4ui.rc DESTINATION ${DATA_INSTALL_DIR}/kcontrol4 )
