project(systemsettings)
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

set(SYSTEMSETTINGS_VERSION 4.9.97)
set(SYSTEMSETTINGS_STRING_VERSION "${SYSTEMSETTINGS_VERSION}")

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
)

find_package(KF5 CONFIG REQUIRED COMPONENTS
    ItemViews
    KCMUtils
    I18n
    KIO
    Service
    IconThemes
    WindowSystem
    XmlGui
    DBusAddons
    Config
    KHtml
    KDE4Support
)

add_subdirectory(core)
add_subdirectory(app)
add_subdirectory(categories)
add_subdirectory(classic)
add_subdirectory(icons)
