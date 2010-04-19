
install( FILES systemsettingscategory.desktop DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

install( FILES 
		 settings-application-and-system-notifications.desktop
		 settings-application-appearance.desktop
		 settings-shortcuts-and-gestures.desktop
		 settings-personal-information.desktop
		 settings-locale.desktop
		 settings-hardware.desktop
		 settings-application-appearance-and-behavior.desktop
		 settings-application-and-system-notifications.desktop
		 settings-account-details.desktop
		 settings-audio-and-video.desktop
		 settings-network-and-connectivity.desktop
		 settings-system-administration.desktop
		 settings-permissions.desktop
		 settings-security.desktop
		 settings-startup-and-shutdown.desktop
		 settings-desktop-appearance.desktop
		 settings-workspace-appearance-and-behavior.desktop
		 settings-input-devices.desktop
		 settings-removable-devices.desktop

         settings-display.desktop
         settings-general.desktop
         settings-accessibility.desktop
         settings-notifications.desktop
         settings-window-behaviour.desktop
         settings-network-settings.desktop
         settings-lost-and-found.desktop
         DESTINATION  ${SERVICES_INSTALL_DIR} )

if (NOT WIN32)
    install( FILES 
         settings-bluetooth.desktop
         #settings-desktop.desktop
         settings-sharing.desktop
         DESTINATION  ${SERVICES_INSTALL_DIR} )
endif (NOT WIN32)
        
