set(systemsettingsview_LIB_SRCS
    MenuItem.cpp
    MenuModel.cpp
    MenuProxyModel.cpp
    ModuleView.cpp
    BaseData.cpp
    BaseMode.cpp
    ExternalAppModule.cpp
)

KDE4_ADD_UI_FILES( systemsettingsview_LIB_SRCS externalModule.ui )

set(systemsettingsview_LIB_HDRS
    systemsettingsview_export.h
    MenuItem.h
    MenuModel.h
    MenuProxyModel.h
    BaseData.h
    BaseMode.h
    ModuleView.h
)

kde4_add_library( systemsettingsview SHARED ${systemsettingsview_LIB_SRCS} )

target_link_libraries( systemsettingsview ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBRARY} ${KDE4_KCMUTILS_LIBS} )

set_target_properties( systemsettingsview PROPERTIES SOVERSION 2 )

install( TARGETS systemsettingsview  ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES ${systemsettingsview_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/systemsettingsview COMPONENT Devel )
INSTALL( FILES systemsettingsview.desktop systemsettingsexternalapp.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
