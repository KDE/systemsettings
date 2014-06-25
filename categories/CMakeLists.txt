
install( FILES systemsettingscategory.desktop DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

install( FILES 
    settings-appearance.desktop
    settings-appearance-themes.desktop
    settings-appearance-style.desktop
    settings-appearance-color.desktop
    settings-appearance-font.desktop
    settings-appearance-icons.desktop

    settings-workspace.desktop
    settings-workspace-windowbehavior.desktop
    settings-workspace-desktopbehavior.desktop
    settings-workspace-shortcuts.desktop
    settings-workspace-session.desktop
#                settings-accessibility.desktop
#                settings-account-details.desktop
#                settings-application-and-system-notifications.desktop
#                settings-application-appearance-and-behavior.desktop
#                settings-application-appearance.desktop
#                settings-audio-and-video.desktop
#                settings-desktop-appearance.desktop
#                settings-display.desktop
#                settings-hardware.desktop
#                settings-input-devices.desktop
#                settings-locale.desktop
#                settings-lost-and-found.desktop
#                settings-network-and-connectivity.desktop
#                settings-network-settings.desktop
#                settings-permissions.desktop
#                settings-personal-information.desktop
#                settings-power-management.desktop
#                settings-removable-devices.desktop
#                settings-shortcuts-and-gestures.desktop
#                settings-startup-and-shutdown.desktop
#                settings-system-administration.desktop
#                settings-window-behaviour.desktop
#                settings-workspace-appearance-and-behavior.desktop
#                settings-workspace-behavior.desktop
         DESTINATION  ${SERVICES_INSTALL_DIR} )

if (NOT WIN32)
    install( FILES 
#          settings-bluetooth.desktop
#          #settings-desktop.desktop
#          settings-sharing.desktop
         DESTINATION  ${SERVICES_INSTALL_DIR} )
endif ()
        
