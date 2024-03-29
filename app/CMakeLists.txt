
add_executable(systemsettings)
ecm_add_qml_module(systemsettings URI org.kde.systemsettings GENERATE_PLUGIN_SOURCE)

target_sources(systemsettings PRIVATE
    main.cpp
    SettingsBase.cpp
    SettingsBase.h
    tooltipmanager.cpp
    tooltipmanager.h
    SidebarMode.cpp
    SidebarMode.h
    ${debug_SRCS}
)

ecm_target_qml_sources(systemsettings SOURCES
    qml/CategoriesPage.qml
    qml/CategoryItem.qml
    qml/HamburgerMenuButton.qml
    qml/IntroPage.qml
    qml/Main.qml
    qml/SideBarItem.qml
    qml/SubCategoryPage.qml
    RESOURCES
    images/plasma-logo.svg
)

qt_target_qml_sources(systemsettings RESOURCES systemsettingsui.rc PREFIX /kxmlgui5/systemsettings)

target_compile_definitions(systemsettings PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(systemsettings PRIVATE
    systemsettings_debug
    systemsettingsview
    KF6::Crash
    KF6::ItemViews
    KF6::ItemModels
    KF6::KCMUtils
    KF6::I18n
    KF6::IconThemes
    KF6::KIOWidgets
    KF6::Service
    KF6::WindowSystem
    KF6::XmlGui
    KF6::DBusAddons
    KF6::ConfigGui
    KF6::GuiAddons # UrlHanlder handles help:/ urls
    Qt::QuickWidgets
    Qt::Qml
    Qt::Quick
)

install(TARGETS systemsettings ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kdesystemsettings.desktop systemsettings.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(DIRECTORY DESTINATION "${KDE_INSTALL_FULL_DATAROOTDIR}/kglobalaccel")
install(
    CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink \"${KDE_INSTALL_FULL_APPDIR}/systemsettings.desktop\" \"\$ENV{DESTDIR}${KDE_INSTALL_FULL_DATAROOTDIR}/kglobalaccel/systemsettings.desktop\")"
)
install(FILES org.kde.systemsettings.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES systemsettings.zsh RENAME _systemsettings DESTINATION ${KDE_INSTALL_ZSHAUTOCOMPLETEDIR})

ecm_finalize_qml_module(systemsettings)
