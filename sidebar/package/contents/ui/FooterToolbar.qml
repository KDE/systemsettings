/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
   SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.15
import QtQuick.Controls 2.5 as QQC2

import org.kde.systemsettings 1.0

QQC2.ToolBar {
    // Match height of SystemSettings-provided footer for KCMs
    // The magic number 13 comes from adding the following:
    // - 6 * 2 for 6px Breeze top and bottom margin values
    // - 1 to account for the height of the separator line in the toolbar
    // FIXME: get margin values from the QStyle instead of hardcoding them
    // TODO: remove this sizer button if System Settings is ever changed to
    //       use toolbuttons instead of pushbuttons, as then the heights will
    //       automatically be equal
    height: sizerButton.height + 13
    QQC2.Button {
        id: sizerButton
        text: "I don't exist"
        icon.name: "edit-bomb"
        visible: false
    }

    QQC2.ToolButton {
        anchors.fill: parent

        text: i18nc("Action to show indicators for settings with custom data. Use as short a translation as is feasible, as horizontal space is limited.", "Highlight Changed Settings")
        icon.name: "draw-highlight"

        onToggled: systemsettings.toggleDefaultsIndicatorsVisibility()
        checkable: true
        checked: systemsettings.defaultsIndicatorsVisible
    }
}
