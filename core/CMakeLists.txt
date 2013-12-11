set(systemsettingsview_LIB_SRCS
    MenuItem.cpp
    MenuModel.cpp
    MenuProxyModel.cpp
    ModuleView.cpp
    BaseData.cpp
    BaseMode.cpp
    ExternalAppModule.cpp
)

kde4_add_ui_files( systemsettingsview_LIB_SRCS externalModule.ui )

set(systemsettingsview_LIB_HDRS
    systemsettingsview_export.h
    MenuItem.h
    MenuModel.h
    MenuProxyModel.h
    BaseData.h
    BaseMode.h
    ModuleView.h
)

add_library( systemsettingsview SHARED ${systemsettingsview_LIB_SRCS} )

target_link_libraries( systemsettingsview
    KF5::ItemViews
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOWidgets
    KF5::Service
    KF5::IconThemes
 )

set_target_properties( systemsettingsview PROPERTIES SOVERSION 2 )

install( TARGETS systemsettingsview  ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES ${systemsettingsview_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/systemsettingsview COMPONENT Devel )
install( FILES systemsettingsview.desktop systemsettingsexternalapp.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
