project(systemsettings)
set(PROJECT_VERSION "5.10.95")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.7.0")
set(KF5_MIN_VERSION "5.30")

find_package(ECM 1.2.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(GenerateExportHeader)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)


find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets Quick Qml QuickWidgets
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    ItemViews
    KCMUtils
    I18n
    KIO
    Service
    IconThemes
    WidgetsAddons
    WindowSystem
    XmlGui
    DBusAddons
    Config
    DocTools
    Package
    Declarative
    Activities
    ActivitiesStats
    OPTIONAL_COMPONENTS
    KHtml
    QUIET
)

find_package(KF5Kirigami2 2.1 REQUIRED)

add_subdirectory(core)
add_subdirectory(app)
add_subdirectory(categories)
add_subdirectory(icons)
add_subdirectory(sidebar)
add_subdirectory(doc)

if(KF5KHtml_FOUND)
    add_subdirectory(classic)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

