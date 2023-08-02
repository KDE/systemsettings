set(systemsettingsview_LIB_SRCS
    BaseData.cpp
    BaseData.h
    BaseMode.cpp
    BaseMode.h
    ExternalAppModule.cpp
    ExternalAppModule.h
    kcmmetadatahelpers.h
    MenuItem.cpp
    MenuItem.h
    MenuModel.cpp
    MenuModel.h
    MenuProxyModel.cpp
    MenuProxyModel.h
    ModuleView.cpp
    ModuleView.h
)

ki18n_wrap_ui(systemsettingsview_LIB_SRCS externalModule.ui)

add_library(systemsettingsview ${systemsettingsview_LIB_SRCS})
generate_export_header(systemsettingsview)

target_link_libraries(systemsettingsview
    KF6::ItemViews
    KF6::KCMUtils
    KF6::I18n
    KF6::KIOWidgets
    KF6::Service
    KF6::IconThemes
    KF6::Activities
    KF6::AuthCore
    KF6::Kirigami2
)

set_target_properties(systemsettingsview PROPERTIES SOVERSION 3)

install(TARGETS systemsettingsview ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
