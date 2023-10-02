install(FILES
    settings-root-category.desktop

    settings-network.desktop
    settings-network-networksettings.desktop
    settings-network-connectivity.desktop

    settings-regionalsettings.desktop
    settings-regionalsettings-formats.desktop

    settings-hardware-input.desktop
    settings-hardware-input-touchscreen.desktop
    settings-hardware-multimedia.desktop
    settings-hardware-display.desktop

    settings-hardware.desktop
    settings-hardware-keyboard.desktop
    settings-hardware-removable-storage.desktop

    settings-appearance.desktop
    settings-appearance-themes.desktop
    settings-appearance-font.desktop

    settings-applications.desktop
    settings-applications-defaults.desktop
    settings-workspace-windowmanagement.desktop

    settings-personalization.desktop
    settings-workspace.desktop
    settings-workspace-desktopbehavior.desktop
    settings-workspace-search.desktop
    settings-workspace-shortcuts.desktop

    settings-security-privacy.desktop
    settings-applications-permissions.desktop

    settings-system-administration.desktop
    settings-hardware-powermanagement.desktop
    settings-workspace-session.desktop

    DESTINATION ${KDE_INSTALL_DATADIR}/systemsettings/categories
)
