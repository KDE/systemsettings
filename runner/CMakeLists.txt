# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

kcoreaddons_add_plugin(krunner_systemsettings SOURCES systemsettingsrunner.cpp INSTALL_NAMESPACE "kf5/krunner")
target_link_libraries(krunner_systemsettings
    KF5::CoreAddons
    KF5::KIOGui
    KF5::I18n
    KF5::Notifications
    KF5::Runner
    KF5::Service
    KF5::Activities
)
