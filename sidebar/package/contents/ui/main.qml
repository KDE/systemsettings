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
import org.kde.kirigami 2.0 as Kirigami

Kirigami.PageRow {
    id: root
    initialPage: mainColumn

    Component.onCompleted: searchField.forceActiveFocus()
    Kirigami.ScrollablePage {
        id: mainColumn
        header: Item {
            width: mainColumn.width
            height:searchLayout.implicitHeight + Kirigami.Units.smallSpacing*2
            RowLayout {
                id: searchLayout
                anchors {
                    fill: parent
                    margins: Kirigami.Units.smallSpacing
                }
                QtControls.ToolButton {
                    iconName: "application-menu"
                    menu: QtControls.Menu {
                        id: globalMenu
                        Instantiator {
                            model: systemsettings.globalActions
                            QtControls.MenuItem {
                                text: modelData.text
                                property QtObject action: modelData
                                onTriggered: action.trigger();
                            }
                            onObjectAdded: globalMenu.insertItem(index, object)
                            onObjectRemoved: globalMenu.removeItem(object)
                        }
                    }
                }
                QtControls.TextField {
                    id: searchField
                    focus: true
                    Layout.fillWidth: true
                    placeholderText: i18n("Search...")
                    onTextChanged: systemsettings.categoryModel.filterRegExp = text
                }
            }
            Kirigami.Separator {
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
            }
        }
        background: Rectangle {
            color: Kirigami.Theme.viewBackgroundColor
        }
        ListView {
            id: categoryView
            anchors.fill: parent
            model: systemsettings.categoryModel
            currentIndex: systemsettings.activeCategory
            section {
                property: "categoryDisplayRole"
                delegate: Item {
                    width: categoryView.width
                    height: sectionLabel.height * 1.6
                    Kirigami.Separator {
                        anchors {
                            left: parent.left
                            right: parent.right
                            bottom: sectionLabel.top
                        }
                        visible: parent.y > 0
                    }
                    Kirigami.Label {
                        anchors {
                            bottom: parent.bottom
                            left: parent.left
                            leftMargin: Kirigami.Units.smallSpacing
                        }
                        id: sectionLabel
                        text: section
                        opacity: 0.3
                        //FIXME: kirigami bug, why?
                        Component.onCompleted: font.bold = true
                    }
                }
            }

            delegate: Kirigami.BasicListItem {
                id: delegate
                icon: model.decoration
                label: model.display
                separatorVisible: false
                activeFocusOnTab: root.currentIndex == 0
                highlighted: focus
                onClicked: {
                    if (systemsettings.activeCategory == index) {
                        root.currentIndex = 1;
                    } else {
                        systemsettings.activeCategory = index;
                    }
                }
                checked: systemsettings.activeCategory == index
                Keys.onPressed: {
                    switch (event.key) {
                    case Qt.Key_Up:
                        delegate.nextItemInFocusChain(false).forceActiveFocus();
                        break;
                    case Qt.Key_Down:
                        delegate.nextItemInFocusChain(true).forceActiveFocus();
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }

    Kirigami.ScrollablePage {
        id: subCategoryColumn
        header: RowLayout {
            QtControls.ToolButton {
                iconName: "go-previous"
                onClicked: root.currentIndex = 0;
            }
            Kirigami.Heading {
                text: "Subcategory"
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
                    root.push(subCategoryColumn);
                    root.currentIndex = 1;
                    subCategoryView.forceActiveFocus();
                } else {
                    root.pop(mainColumn)
                }
            }

            delegate: Kirigami.BasicListItem {
                id: delegate
                icon: model.decoration
                label: model.display
                separatorVisible: false
                activeFocusOnTab: root.currentIndex == 1
                highlighted: focus
                onClicked: systemsettings.activeSubCategory = index
                checked: systemsettings.activeSubCategory == index
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
                        root.currentIndex = 0;
                        mainColumn.forceActiveFocus();
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
}
