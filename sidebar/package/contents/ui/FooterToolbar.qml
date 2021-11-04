/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.15
import QtQuick.Controls 2.5 as QQC2

import org.kde.systemsettings 1.0

QQC2.ToolBar {
    QQC2.ToolButton {
        anchors.fill: parent

        text: i18nc("Action to show indicators for settings with custom data. Use as short a translation as is feasible, as horizontal space is limited.", "Highlight Changed Settings")
        icon.name: "draw-highlight"

        onToggled: systemsettings.toggleDefaultsIndicatorsVisibility()
        checkable: true
        checked: systemsettings.defaultsIndicatorsVisible
    }
}
