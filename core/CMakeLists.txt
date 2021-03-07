set(systemsettingsview_LIB_SRCS
    MenuItem.cpp
    MenuModel.cpp
    MenuProxyModel.cpp
    ModuleView.cpp
    BaseData.cpp
    BaseMode.cpp
    ExternalAppModule.cpp
)

ki18n_wrap_ui( systemsettingsview_LIB_SRCS externalModule.ui )

add_library( systemsettingsview ${systemsettingsview_LIB_SRCS} )
generate_export_header(systemsettingsview)

target_link_libraries( systemsettingsview
    KF5::ItemViews
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOWidgets
    KF5::Service
    KF5::IconThemes
    KF5::Activities
 )

set_target_properties( systemsettingsview PROPERTIES SOVERSION 3 )

install( TARGETS systemsettingsview ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
install( FILES systemsettingsview.desktop systemsettingsexternalapp.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR} )
