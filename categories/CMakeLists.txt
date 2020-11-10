
install( FILES systemsettingscategory.desktop DESTINATION  ${KDE_INSTALL_KSERVICETYPES5DIR} )

install( FILES 
    settings-appearance.desktop
    settings-appearance-color.desktop
    settings-appearance-font.desktop
    settings-appearance-icons.desktop
    settings-root-category.desktop

    settings-workspace.desktop
    settings-workspace-windowmanagement.desktop
    settings-workspace-desktopbehavior.desktop
    settings-workspace-shortcuts.desktop
    settings-workspace-session.desktop
    settings-workspace-search.desktop

    settings-personalization.desktop
    settings-personalization-accountdetails.desktop
    settings-personalization-accessibility.desktop
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

    settings-system-administration.desktop

    DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )

if (NOT WIN32)
    install( FILES
         settings-network-bluetooth.desktop
         DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
endif ()

