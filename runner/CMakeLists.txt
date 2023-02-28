# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

kcoreaddons_add_plugin(krunner_systemsettings SOURCES systemsettingsrunner.cpp 
    ${debug_SRCS}  systemsettingsrunner.h INSTALL_NAMESPACE kf${QT_MAJOR_VERSION}/krunner)
target_link_libraries(krunner_systemsettings
    KF6::CoreAddons
    KF6::KIOGui
    KF6::I18n
    KF6::Notifications
    KF6::Runner
    KF6::Service
    KF6::Activities
    KF6::KCMUtils
)
