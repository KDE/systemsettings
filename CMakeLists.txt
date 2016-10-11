project(systemsettings)
set(PROJECT_VERSION "5.8.1")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.4.0")

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

find_package(ECM 1.2.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(GenerateExportHeader)


find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
)

find_package(KF5 REQUIRED COMPONENTS
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
    DocTools
    OPTIONAL_COMPONENTS
    KHtml
    QUIET
)

add_subdirectory(core)
add_subdirectory(app)
add_subdirectory(categories)
add_subdirectory(icons)
add_subdirectory(doc)

if(KF5KHtml_FOUND)
    add_subdirectory(classic)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

