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

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Controls 2.0 as QtControls2
import org.kde.kirigami 2.1 as Kirigami


Kirigami.ScrollablePage {
    id: subCategoryColumn
    header: MouseArea {
        width: subCategoryColumn.width
        height: Kirigami.Units.gridUnit * 2.5
        enabled: !applicationWindow().wideScreen
        onClicked: root.pageStack.currentIndex = 0;
        Accessible.role: Accessible.Button
        Accessible.name: i18n("Back")

        Item {
            id: headerControls

            anchors.fill: parent

            QtControls.ToolButton {
                id: backButton
                anchors.fill: parent
                visible: !applicationWindow().wideScreen
                onClicked: root.pageStack.currentIndex = 0;

                RowLayout {
                    anchors.fill: parent

                    Kirigami.Icon {
                        id: toolButtonIcon
                        Layout.alignment: Qt.AlignVCenter
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        Layout.preferredWidth: Layout.preferredHeight

                        source: LayoutMirroring.enabled ? "go-next" : "go-previous"
                    }

                    QtControls2.Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        height: toolButtonIcon.height
                        text: subCategoryColumn.title
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        //FIXME: QtControls bug, why?
                        Component.onCompleted: font.bold = true
                    }
                }
            }
            QtControls2.Label {
                anchors.verticalCenter: parent.verticalCenter
                x: y

                text: subCategoryColumn.title
                elide: Text.ElideRight
                visible: !backButton.visible
                opacity: 0.3
                //FIXME: QtControls bug, why?
                Component.onCompleted: font.bold = true
            }
        }
        Kirigami.Separator {
            visible: !subCategoryView.atYBeginning
            anchors {
                left: parent.left
                right: parent.right
                top: parent.bottom
            }
        }
    }
    background: Rectangle {
        color: Kirigami.Theme.viewBackgroundColor
    }
    ListView {
        id: subCategoryView
        anchors.fill: parent
        model: systemsettings.subCategoryModel
        currentIndex: systemsettings.activeSubCategory
        activeFocusOnTab: true
        keyNavigationWraps: true
        Accessible.role: Accessible.List
        Keys.onTabPressed: root.focusNextRequest();
        Keys.onBacktabPressed: {
            mainColumn.focus = true;
        }
        onCountChanged: {
            if (count > 1) {
                if (root.pageStack.depth < 2) {
                    root.pageStack.push(subCategoryColumn);
                }
            } else {
                root.pageStack.pop(mainColumn)
            }
        }
        Connections {
            target: systemsettings
            onActiveSubCategoryChanged: {
                root.pageStack.currentIndex = 1;
                subCategoryView.forceActiveFocus();
            }
        }

        delegate: Kirigami.BasicListItem {
            id: delegate
            icon: model.decoration
            label: model.display
            separatorVisible: false
            highlighted: focus
            onClicked: systemsettings.activeSubCategory = index
            onFocusChanged: {
                if (focus) {
                    onCurrentIndexChanged: subCategoryView.positionViewAtIndex(index, ListView.Contain);
                }
            }
            checked: systemsettings.activeSubCategory == index
            //checkable: false
            //FIXME: Qt 5.7 doesn't have checkable, this way fails at runtime but still works correctly on 5.7
            Component.onCompleted: delegate.checkable = true;
        }
    }
}
