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
    required property QtObject /*QAction*/ auxiliaryAction

    width: ListView.view?.width ?? 0

    Accessible.name: text
    Accessible.onPressAction: clicked()

    contentItem: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.IconTitleSubtitle {
            id: titleItem
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

        Component {
            id: auxiliaryButtonActionComponent

            ToolButton {
                implicitWidth: height
                implicitHeight: titleItem.height
                icon.color: delegate.selected || pressed || visualFocus ? palette.highlight : palette.buttonText

                display: AbstractButton.IconOnly
                text: delegate.auxiliaryAction.text
                icon.name: systemsettings.actionIconName(delegate.auxiliaryAction)
                onClicked: {
                    delegate.auxiliaryAction.trigger();
                }

                ToolTip.text: delegate.auxiliaryAction.tooltip || delegate.auxiliaryAction.text
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.visible: ToolTip.text !== "" && (Kirigami.Settings.tabletMode ? pressed : hovered)
            }
        }

        Component {
            id: auxiliarySwitchActionComponent

            Switch {
                Accessible.name: delegate.auxiliaryAction.text
                checked: delegate.auxiliaryAction.checked
                onToggled: {
                    delegate.auxiliaryAction.trigger();
                }

                ToolTip.text: delegate.auxiliaryAction.tooltip || delegate.auxiliaryAction.text
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.visible: ToolTip.text !== "" && (Kirigami.Settings.tabletMode ? pressed : hovered)
            }
        }

        Loader {
            Layout.fillHeight: true
            Layout.topMargin: -delegate.topPadding + delegate.topInset
            Layout.bottomMargin: -delegate.bottomPadding + delegate.bottomInset
            Layout.rightMargin: -delegate.rightPadding + delegate.rightInset

            enabled: delegate.auxiliaryAction?.enabled ?? false
            visible: status === Loader.Ready
            sourceComponent: {
                const action = delegate.auxiliaryAction;
                if (action && action.visible) {
                    if (action.checkable) {
                        return auxiliarySwitchActionComponent;
                    } else {
                        return auxiliaryButtonActionComponent;
                    }
                }
                return null;
            }
        }

        Kirigami.Icon {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
            Layout.preferredHeight: Kirigami.Units.iconSizes.small

            opacity: delegate.showArrow ? 0.7 : 0.0
            source: LayoutMirroring.enabled ? "go-next-symbolic-rtl" : "go-next-symbolic"
            selected: delegate.selected
            visible: !delegate.auxiliaryAction?.visible ?? true
        }
    }
}
