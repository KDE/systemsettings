
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

         settings-advanced.desktop
         settings-advanced-user-settings.desktop
         settings-computer-administration.desktop
         settings-display.desktop
         settings-general.desktop
         settings-look-and-feel.desktop
         settings-network-and-connectivity.desktop
         settings-personal.desktop
         settings-system.desktop
         settings-about-me.desktop
         settings-accessibility.desktop
         settings-keyboard-and-mouse.desktop
         settings-notifications.desktop
         settings-regional-and-language.desktop
         settings-window-behaviour.desktop
         settings-network-settings.desktop
         settings-lost-and-found.desktop
		 settings-hardware.desktop
         DESTINATION  ${SERVICES_INSTALL_DIR} )

if (NOT WIN32)
    install( FILES 
         settings-bluetooth.desktop
         settings-desktop.desktop
         settings-sharing.desktop
         DESTINATION  ${SERVICES_INSTALL_DIR} )
endif (NOT WIN32)
        
