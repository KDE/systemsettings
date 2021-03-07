/*
   Copyright (c) 2017 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.5 as Kirigami
import org.kde.systemsettings 1.0

Kirigami.ScrollablePage {
    id: subCategoryColumn
    title: systemsettings.subCategoryModel.title

    header: Kirigami.AbstractApplicationHeader {
        id: pageHeader

        topPadding: Kirigami.Units.smallSpacing
        bottomPadding: Kirigami.Units.smallSpacing
        leftPadding: Kirigami.Units.smallSpacing
        rightPadding: Kirigami.Units.smallSpacing

        implicitHeight: topPadding + sizeHelper.implicitHeight + bottomPadding

        // Not visible; just to get its size so we can match this custom header
        // with the height of a standard header
        Kirigami.Heading {
            id: sizeHelper
            // otherwise it gets parented to the content item which we don't want
            parent: pageHeader
            text: "Placeholder"
            visible: false
        }

        contentItem: RowLayout {
            id: toolBarLayout
            anchors.fill: parent
            spacing: 0

            QQC2.ToolButton {
                id: backButton
                visible: !applicationWindow().wideScreen
                icon.name: LayoutMirroring.enabled ? "go-previous-symbolic-rtl" : "go-previous-symbolic"
                onClicked: root.pageStack.currentIndex = 0;
                Accessible.role: Accessible.Button
                Accessible.name: i18n("Go back")
                QQC2.ToolTip {
                    text: parent.Accessible.name
                }
            }

            MouseArea {
                id: backToCategoryButton
                implicitWidth: titleLayout.implicitWidth
                implicitHeight: titleLayout.implicitHeight
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.smallSpacing
                enabled: systemsettings.subCategoryModel.categoryOwnedByKCM
                hoverEnabled: true
                activeFocusOnTab: true
                Accessible.role: Accessible.Button
                Accessible.name: subCategoryColumn.title
                onClicked: {
                    focus = true;
                    systemsettings.subCategoryModel.loadParentCategoryModule();
                }
                Rectangle {
                    anchors {
                        fill: parent
                        leftMargin: -Kirigami.Units.smallSpacing
                    }
                    radius: Kirigami.Units.smallSpacing
                    color: Kirigami.Theme.highlightColor
                    opacity: parent.containsMouse ? (parent.pressed ? 0.4 : 0.2) : (parent.activeFocus ? 0.1 : 0)
                    Behavior on opacity {
                        OpacityAnimator {
                            duration: Kirigami.Units.shortDuration
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
                RowLayout {
                    id: titleLayout
                    spacing: Kirigami.Units.smallSpacing
                    Kirigami.Icon {
                        visible: systemsettings.subCategoryModel.categoryOwnedByKCM
                        source: systemsettings.subCategoryModel.icon
                        implicitWidth: Kirigami.Units.iconSizes.smallMedium
                        implicitHeight: Kirigami.Units.iconSizes.smallMedium
                    }
                    Kirigami.Heading {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        // Don't be too short when the back button isn't visible
                        Layout.minimumHeight: backButton.implicitHeight
                        level: 3
                        text: subCategoryColumn.title
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
    background: Rectangle {
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        color: Kirigami.Theme.backgroundColor
    }
    ListView {
        id: subCategoryView
        anchors.fill: parent
        model: systemsettings.subCategoryModel
        currentIndex: systemsettings.activeSubCategoryRow
        onContentYChanged: systemsettings.hideToolTip();
        activeFocusOnTab: true
        keyNavigationWraps: true
        Accessible.role: Accessible.List
        Keys.onTabPressed: root.focusNextRequest();
        Keys.onBacktabPressed: {
            mainColumn.focus = true;
        }
        onCountChanged: {
            if (count > 1 && !root.searchMode) {
                if (root.pageStack.depth < 2) {
                    root.pageStack.push(subCategoryColumn);
                }
            } else {
                root.pageStack.pop(mainColumn)
            }
        }

        Connections {
            target: root
            onSearchModeChanged: {
                if (root.searchMode) {
                    root.pageStack.pop(mainColumn);
                } else if (subCategoryView.count > 1) {
                    root.pageStack.push(subCategoryColumn);
                }
            }
        }
        Connections {
            target: systemsettings
            onActiveSubCategoryRowChanged: {
                if (systemsettings.activeSubCategoryRow >= 0) {
                    root.pageStack.currentIndex = 1;
                    subCategoryView.forceActiveFocus();
                } else {
                    backToCategoryButton.forceActiveFocus();
                }
            }
            onIntroPageVisibleChanged: {
                if (systemsettings.introPageVisible) {
                    root.pageStack.pop(mainColumn)
                }
            }
        }

        delegate: Kirigami.AbstractListItem {
            id: delegate
            onClicked: {
                systemsettings.loadModule(subCategoryView.model.index(index, 0));
            }
            onHoveredChanged: {
                if (hovered) {
                    systemsettings.requestToolTip(subCategoryView.model.index(index, 0), delegate.mapToItem(root, 0, 0, width, height));
                } else {
                    systemsettings.hideToolTip();
                }
            }
            onFocusChanged: {
                if (focus) {
                    onCurrentIndexChanged: subCategoryView.positionViewAtIndex(index, ListView.Contain);
                }
            }
            highlighted: systemsettings.activeSubCategoryRow == index
            Keys.onEnterPressed: clicked();
            Keys.onReturnPressed: clicked();
            contentItem: CategoryItem {}
        }
    }

    footer: Rectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        width: mainColumn.width
        height: Kirigami.Units.gridUnit * 2
        visible: systemsettings.applicationMode == SystemSettings.SystemSettings &&
                     !applicationWindow().wideScreen
        QQC2.ToolButton {
            anchors {
                fill: parent
                margins: Kirigami.Units.smallSpacing
            }
            text: i18nc("Action to show indicators for settings with custom data", "Highlight Changed Settings")
            icon.name: "draw-highlight"
            onToggled: systemsettings.toggleDefaultsIndicatorsVisibility()
            checkable: true
            checked: systemsettings.defaultsIndicatorsVisible
        }

        Kirigami.Separator {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
        }
    }
}
