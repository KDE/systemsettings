cmake_minimum_required(VERSION 3.0)

set(PROJECT_VERSION "5.18.7")

project(systemsettings VERSION ${PROJECT_VERSION})
set(QT_MIN_VERSION "5.12.0")
set(KF5_MIN_VERSION "5.65.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(GenerateExportHeader)
include(ECMQMLModules)
include(ECMQtDeclareLoggingCategory)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)


find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets Quick Qml QuickWidgets
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
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
    DocTools
    Package
    Declarative
    Activities
    ActivitiesStats
    QUIET
)

ecm_find_qmlmodule(org.kde.kcm 1.0)

find_package(KF5Kirigami2 2.1 CONFIG REQUIRED)

find_package(LibKWorkspace ${PROJECT_VERSION} CONFIG REQUIRED)

if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
   add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)
endif()

add_subdirectory(core)
add_subdirectory(app)
add_subdirectory(categories)
add_subdirectory(icons)
add_subdirectory(sidebar)
add_subdirectory(doc)

install(FILES systemsettings.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

