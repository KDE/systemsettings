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
    header: Item {
        width: subCategoryColumn.width
        height: topLayout.implicitHeight + Kirigami.Units.smallSpacing * 2
        RowLayout {
            id: topLayout
            height: backButton.height
            spacing: Kirigami.Units.smallSpacing
            anchors {
                fill: parent
                margins: Kirigami.Units.smallSpacing
                leftMargin: backButton.visible ? Kirigami.Units.smallSpacing : Kirigami.Units.smallSpacing * 2
            }
            QtControls.ToolButton {
                id: backButton
                Layout.maximumWidth: Kirigami.Units.iconSizes.smallMedium + Kirigami.Units.smallSpacing * 2
                Layout.maximumHeight: width
                visible: !applicationWindow().wideScreen
                iconName: "go-previous"
                onClicked: root.pageStack.currentIndex = 0;
            }
            Kirigami.Label {
                Layout.fillWidth: true
                Layout.minimumHeight: Layout.maximumHeight
                Layout.maximumHeight: Kirigami.Units.iconSizes.smallMedium + Kirigami.Units.smallSpacing * 2
                text: subCategoryColumn.title
                elide: Text.ElideRight
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
                root.pageStack.currentIndex = 1;
                subCategoryView.forceActiveFocus();
            } else {
                root.pageStack.pop(mainColumn)
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
