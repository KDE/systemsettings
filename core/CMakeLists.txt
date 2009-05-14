include_directories( ToolTips )

set(systemsettingsview_LIB_SRCS
    MenuItem.cpp
    MenuModel.cpp
    MenuProxyModel.cpp
    ModuleView.cpp
    BaseData.cpp
    BaseMode.cpp
    ToolTips/SystemSettingsBalloonToolTipDelegate.cpp
    ToolTips/KFormattedBalloonTipDelegate.cpp
    ToolTips/SystemSettingsToolTipItem.cpp
    ToolTips/KStyleOptionToolTip.cpp
    ToolTips/KTipLabel.cpp
    ToolTips/KToolTip.cpp
    ToolTips/KToolTipDelegate.cpp
    ToolTips/KToolTipItem.cpp
    ToolTips/KToolTipManager.cpp
    ToolTipManager.cpp
)

set(systemsettingsview_LIB_HDRS
    systemsettingsview_export.h
    MenuItem.h
    MenuModel.h
    MenuProxyModel.h
    BaseMode.h
    ModuleView.h
    ToolTipManager.h
)

kde4_add_library( systemsettingsview SHARED ${systemsettingsview_LIB_SRCS} )

target_link_libraries( systemsettingsview ${KDE4_KDECORE_LIBS} ${KDE4_KUTILS_LIBS} )

install( TARGETS systemsettingsview  ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES ${systemsettingsview_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/systemsettingsview COMPONENT Devel )
INSTALL( FILES systemsettingsview.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
