/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
   SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.15
import QtQuick.Controls 2.5 as QQC2

import org.kde.qqc2desktopstyle.private 1.0 as Style
import org.kde.systemsettings 1.0

QQC2.ToolBar {
    // Match height and padding of SystemSettings-provided footer for KCMs
    Style.StyleItem {
        id: desktopStyle
    }
    // + 1 on top to account for the height of the separator line in the toolbar
    topPadding:    desktopStyle.pixelMetric("layouttopmargin") + 1
    bottomPadding: desktopStyle.pixelMetric("layoutbottommargin")
    leftPadding:   desktopStyle.pixelMetric("layoutleftmargin")
    rightPadding:  desktopStyle.pixelMetric("layoutrightmargin")

    // TODO: remove this sizer button if System Settings is ever changed to
    //       use toolbuttons instead of pushbuttons, as then the heights will
    //       automatically be equal
    height: sizerButton.height + topPadding + bottomPadding
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
