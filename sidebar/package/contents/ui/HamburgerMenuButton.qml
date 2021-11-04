/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.15
import QtQuick.Controls 2.5 as QQC2

import org.kde.systemsettings 1.0

QQC2.ToolButton {
    icon.name: "application-menu"
    checkable: true
    checked: systemsettings.actionMenuVisible
    onClicked: systemsettings.showActionMenu(mapToGlobal(0, height))

    Accessible.name: i18n("Show menu")
    QQC2.ToolTip {
        text: parent.Accessible.name
        // Close when menu is open, or else it gets cut off behind
        visible: parent.hovered & !parent.checked
    }
}
