/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
   SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.20 as Kirigami

import org.kde.systemsettings 1.0

QQC2.ToolButton {
    id: control

    // This is needed because more than one hamburger button can exist in a scene.
    property bool ownsVisibleActionMenu: false

    icon.name: "application-menu"

    checkable: true
    checked: systemsettings.actionMenuVisible
    onToggled: if (checked) {
        ownsVisibleActionMenu = true;
        systemsettings.showActionMenu(mapToGlobal(0, height));
    }

    Connections {
        target: systemsettings
        function onActionMenuVisibleChanged() {
            if (control.ownsVisibleActionMenu) {
                ownsVisibleActionMenu = false;
                control.forceActiveFocus(Qt.PopupFocusReason);
            }
        }
    }

    Accessible.role: Accessible.ButtonMenu
    Accessible.name: i18n("Show menu")
    QQC2.ToolTip.text: Accessible.name
    QQC2.ToolTip.visible: hovered && !down
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
}
