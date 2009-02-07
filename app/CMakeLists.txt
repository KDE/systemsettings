SET(kcontrol4_SRCS
    SettingsBase.cpp
    main.cpp )

KDE4_ADD_EXECUTABLE(kcontrol4 ${kcontrol4_SRCS})

TARGET_LINK_LIBRARIES(kcontrol4 ${KDE4_KIO_LIBS} kcontrolview )

INSTALL( TARGETS kcontrol4 DESTINATION ${BIN_INSTALL_DIR} )

