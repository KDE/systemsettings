set(icon_mode_SRCS
    IconMode.cpp
    CategoryDrawer.cpp
    CategorizedView.cpp
)

add_library(icon_mode MODULE ${icon_mode_SRCS})

target_link_libraries(icon_mode systemsettingsview
    KF6::ItemViews
    KF6::KCMUtils
    KF6::I18n
    KF6::KIOWidgets
    KF6::Service
)

install(TARGETS icon_mode DESTINATION ${KDE_INSTALL_PLUGINDIR}/systemsettingsview)
