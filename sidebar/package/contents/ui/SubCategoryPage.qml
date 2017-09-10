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
        height: toolButtonIcon.height + Kirigami.Units.smallSpacing * 4
        enabled: !applicationWindow().wideScreen
        onClicked: root.pageStack.currentIndex = 0;
        Accessible.role: Accessible.Button
        Accessible.name: i18n("Back")

        Item {
            id: headerControls

            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing

            QtControls.ToolButton {
                id: backButton
                anchors.fill: parent
                visible: !applicationWindow().wideScreen
                onClicked: root.pageStack.currentIndex = 0;

                Item {
                    anchors.fill: parent
                    opacity: 0.3

                    Kirigami.Icon {
                        id: toolButtonIcon
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                        height: Kirigami.Units.iconSizes.small
                        width: height
                        source: "go-previous"
                    }

                    Kirigami.Label {
                        anchors {
                            left: toolButtonIcon.right
                            right: parent.right
                            leftMargin: Kirigami.Units.smallSpacing
                            verticalCenter: parent.verticalCenter
                        }
                        height: toolButtonIcon.height
                        text: subCategoryColumn.title
                        elide: Text.ElideRight
                        //FIXME: kirigami bug, why?
                        Component.onCompleted: font.bold = true
                    }
                }
            }
            Kirigami.Label {
                anchors {
                    fill: parent
                    leftMargin: Kirigami.Units.smallSpacing
                }
                text: subCategoryColumn.title
                elide: Text.ElideRight
                visible: !backButton.visible
                opacity: 0.3
                //FIXME: kirigami bug, why?
                Component.onCompleted: font.bold = true
            }
        }
        Kirigami.Separator {
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
            activeFocusOnTab: root.pageStack.currentIndex == 1
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
            Keys.onPressed: {
                switch (event.key) {
                case Qt.Key_Up:
                    delegate.nextItemInFocusChain(false).forceActiveFocus();
                    break;
                case Qt.Key_Down:
                    delegate.nextItemInFocusChain(true).forceActiveFocus();
                    break;
                case Qt.Key_left:
                case Qt.Key_Backspace:
                    root.pageStack.currentIndex = 0;
                    mainColumn.forceActiveFocus();
                    break;
                default:
                    break;
                }
            }
        }
    }
}
