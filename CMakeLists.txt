cmake_minimum_required(VERSION 3.0)

set(PROJECT_VERSION "5.13.80")

project(systemsettings VERSION ${PROJECT_VERSION})
set(QT_MIN_VERSION "5.11.0")
set(KF5_MIN_VERSION "5.50.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(GenerateExportHeader)
include(ECMQMLModules)
include(ECMQtDeclareLoggingCategory)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets Quick Qml QuickWidgets
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Crash
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

ecm_find_qmlmodule(org.kde.kcm 1.0)

find_package(KF5Kirigami2 2.1 REQUIRED)

find_package(LibKWorkspace ${PROJECT_VERSION} REQUIRED)

add_subdirectory(core)
add_subdirectory(app)
add_subdirectory(categories)
add_subdirectory(icons)
add_subdirectory(sidebar)
add_subdirectory(doc)

if(KF5KHtml_FOUND)
    add_subdirectory(classic)
endif()

install(FILES systemsettings.categories DESTINATION ${KDE_INSTALL_CONFDIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

