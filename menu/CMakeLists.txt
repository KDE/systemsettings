FIND_PACKAGE(KDE4 REQUIRED)

SET(CMAKE_VERBOSE_MAKEFILE ON)

ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

LINK_DIRECTORIES(${KDE4_LIB_DIR})

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL_FILES( /share/applications/kde FILES  audioencoding.desktop defaultapplication.desktop kcmfontinst.desktop kcm_knetworkconfmodule_ss.desktop laptoppowermanagement.desktop medianotifications.desktop )

install( FILES audioencoding.desktop defaultapplication.desktop kcmfontinst.desktop kcm_knetworkconfmodule_ss.desktop laptoppowermanagement.desktop medianotifications.desktop  DESTINATION  ${XDG_APPS_DIR} )

install( FILES system-settings.menu  DESTINATION  ${SYSCONF_INSTALL_DIR}/xdg/menus )
install( FILES system-settings-merge.menu  DESTINATION  ${SYSCONF_INSTALL_DIR}/xdg/menus/applications-merged )

install( FILES kde-settings-accessibility_ss.directory  kde-settings-advanced.directory  kde-settings-advancedusersettings.directory  kde-settings-appearance.directory  kde-settings-bluetooth.directory  kde-settings-computeradministration.directory  kde-settings-defaultapplications.directory  kde-settings-general.directory  kde-settings-keyboardmouse.directory  kde-settings-laptops.directory  kde-settings-looknfeel2.directory  kde-settings-networkconnections.directory  kde-settings-networkconnectivity.directory  kde-settings-networksettings.directory  kde-settings-notifications.directory  kde-settings-panel.directory  kde-settings-personal.directory  kde-settings-power.directory  kde-settings-regional.directory  kde-settings-sharing.directory  kde-settings-splashscreen.directory  kde-settings-system.directory  kde-settings-useraccount.directory  kde-settings-windowbehavior.directory  system-settings.directory  DESTINATION  ${XDG_DIRECTORY_DIR} )
