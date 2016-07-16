
install( FILES systemsettingscategory.desktop DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

install( FILES 
    settings-appearance.desktop
    settings-appearance-workspacetheme.desktop
    settings-appearance-applicationstyle.desktop
    settings-appearance-color.desktop
    settings-appearance-font.desktop
    settings-appearance-icons.desktop

    settings-workspace.desktop
    settings-workspace-windowmanagement.desktop
    settings-workspace-desktopbehavior.desktop
    settings-workspace-shortcuts.desktop
    settings-workspace-session.desktop
    settings-workspace-search.desktop

    settings-personalization.desktop
    settings-personalization-accountdetails.desktop
    settings-personalization-regionalsettings.desktop
    settings-personalization-notification.desktop
    settings-personalization-applications.desktop

    settings-network.desktop
    settings-network-networksettings.desktop
    settings-network-connectivity.desktop

    settings-hardware.desktop
    settings-hardware-display.desktop
    settings-hardware-input.desktop
    settings-hardware-multimedia.desktop
    settings-hardware-peripherals.desktop
    settings-hardware-powermanagement.desktop
    settings-hardware-removable-storage.desktop

    DESTINATION  ${SERVICES_INSTALL_DIR} )

if (NOT WIN32)
    install( FILES
         settings-network-bluetooth.desktop
         DESTINATION  ${SERVICES_INSTALL_DIR} )
endif ()

