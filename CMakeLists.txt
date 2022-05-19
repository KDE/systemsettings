cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION "5.25.80")

project(systemsettings VERSION ${PROJECT_VERSION})
set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.87")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(GenerateExportHeader)
include(ECMFindQmlModule)
include(ECMQtDeclareLoggingCategory)
include(KDEGitCommitHooks)

add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055700)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f02)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)


find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets Quick Qml QuickWidgets
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Auth
    Crash
    ItemViews
    ItemModels
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
    Package
    Activities
    ActivitiesStats
    GuiAddons # UrlHanlder handles help:/ urls
    Kirigami2
    Notifications
    Runner
    OPTIONAL_COMPONENTS
    DocTools
    QUIET
)

ecm_find_qmlmodule(org.kde.kcm 1.0)

find_package(LibKWorkspace ${PROJECT_VERSION} CONFIG REQUIRED)

add_subdirectory(core)
add_subdirectory(app)
add_subdirectory(categories)
add_subdirectory(icons)
add_subdirectory(sidebar)
if(KF5DocTools_FOUND)
    add_subdirectory(doc)
endif()
add_subdirectory(runner)

ecm_qt_install_logging_categories(EXPORT SYSTEMSETTINGS FILE systemsettings.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

