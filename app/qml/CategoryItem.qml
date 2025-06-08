/*
 *   SPDX-FileCopyrightText: 2021 Jan Blackquill <uhhadd@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ItemDelegate {
    id: delegate

    property bool showArrow: false
    property bool selected: delegate.highlighted || delegate.pressed
    property bool isSearching: false
    property real leadingPadding: 0
    required property bool showDefaultIndicator
    required property QtObject /*QAction*/ helpfulAction

    width: ListView.view?.width ?? 0

    Accessible.name: text
    Accessible.onPressAction: clicked()

    contentItem: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.IconTitleSubtitle {
            Layout.fillWidth: true
            Layout.leftMargin: delegate.leadingPadding
            icon: icon.fromControlsIcon(delegate.icon)
            title: delegate.text
            selected: delegate.selected
        }

        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: Kirigami.Units.largeSpacing
            Layout.preferredHeight: Kirigami.Units.largeSpacing

            radius: width * 0.5
            visible: delegate.showDefaultIndicator && systemsettings.defaultsIndicatorsVisible
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            color: Kirigami.Theme.neutralTextColor
        }

        // FIXME makes the delegate taller, don't do that.
        ToolButton {
            display: AbstractButton.IconOnly
            text: delegate.helpfulAction?.text ?? ""
            // FIXME cannot use QIcon in QML ........
            //icon.name: auxiliaryAction?.icon.name ?? ""
            enabled: delegate.helpfulAction?.enabled ?? false
            visible: (delegate.helpfulAction?.visible && !delegate.helpfulAction?.checkable) ?? false
            onClicked: {
                delegate.helpfulAction.trigger();
            }

            ToolTip.text: delegate.helpfulAction?.tooltip ?? ""
        }

        Switch {
            id: kcmSwitch
            Accessible.name: delegate.helpfulAction?.text ?? ""
            checked: delegate.helpfulAction?.checked ?? false
            enabled: delegate.helpfulAction?.enabled ?? false
            visible: (delegate.helpfulAction?.visible && delegate.helpfulAction?.checkable) ?? false
            onToggled: {
                delegate.helpfulAction.trigger();
            }

            ToolTip.text: (delegate.helpfulAction?.tooltip || delegate.helpfulAction?.text) ?? ""
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.visible: kcmSwitch.hovered
        }

        Kirigami.Icon {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
            Layout.preferredHeight: Kirigami.Units.iconSizes.small

            opacity: delegate.showArrow ? 0.7 : 0.0
            source: LayoutMirroring.enabled ? "go-next-symbolic-rtl" : "go-next-symbolic"
            selected: delegate.selected
            visible: !delegate.helpfulAction?.visible ?? true
        }
    }
}
