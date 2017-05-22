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

import QtQuick 2.3
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Controls 2.0 as QtControls2
import org.kde.kirigami 2.1 as Kirigami


Kirigami.ScrollablePage {
    id: mainColumn
    Component.onCompleted: searchField.forceActiveFocus()

    header: Item {
        width: mainColumn.width
        height:searchLayout.height + Kirigami.Units.smallSpacing * 2
        RowLayout {
            id: searchLayout
            height: Math.max(menuButton.height, Kirigami.Units.gridUnit * 2)
            anchors {
                fill: parent
                margins: Kirigami.Units.smallSpacing
            }
            QtControls.ToolButton {
                id: menuButton
                iconName: "application-menu"
                menu: QtControls.Menu {
                    id: globalMenu
                    QtControls.MenuItem {
                        text: i18n("Configure")
                        iconName: "settings-configure"
                        onTriggered: systemsettings.triggerGlobalAction("configure");
                    }
                    QtControls.MenuItem {
                        text: i18n("System Settings Handbook")
                        iconName: "help-contents"
                        onTriggered: systemsettings.triggerGlobalAction("help_contents");
                    }
                    QtControls.MenuItem {
                        text: i18n("About System Settings")
                        iconName: "help-about"
                        onTriggered: systemsettings.triggerGlobalAction("help_about_app");
                    }
                    QtControls.MenuItem {
                        text: i18n("About KDE")
                        iconName: "kde"
                        onTriggered: systemsettings.triggerGlobalAction("help_about_kde");
                    }
                }
            }
            QtControls.TextField {
                id: searchField
                focus: true
                Layout.fillWidth: true
                placeholderText: i18n("Search...")
                onTextChanged: {
                    systemsettings.categoryModel.filterRegExp = text;
                }
                MouseArea {
                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        rightMargin: y
                    }
                    opacity: searchField.text.length > 0 ? 1 : 0
                    width: Kirigami.Units.iconSizes.small
                    height: width
                    onClicked: searchField.text = ""
                    Kirigami.Icon {
                        anchors.fill: parent
                        source: LayoutMirroring.enabled ? "edit-clear-rtl" : "edit-clear"
                    }
                    Behavior on opacity {
                        OpacityAnimator {
                            duration: Kirigami.Units.longDuration
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
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
    Kirigami.Heading {
        anchors.centerIn: parent
        width: parent.width * 0.7
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        text: i18nc("A search yelded no results", "No items matching your search")
        opacity: categoryView.count == 0 ? 0.3 : 0
        Behavior on opacity {
            OpacityAnimator {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
    ListView {
        id: categoryView
        anchors.fill: parent
        model: systemsettings.categoryModel
        currentIndex: systemsettings.activeCategory
        onContentYChanged: systemsettings.hideToolTip();
        section {
            property: "categoryDisplayRole"
            delegate: Item {
                width: categoryView.width
                height: Kirigami.Units.iconSizes.smallMedium + Kirigami.Units.smallSpacing * 4
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
                        right: parent.right
                        leftMargin: Kirigami.Units.smallSpacing
                    }
                    id: sectionLabel
                    text: section
                    opacity: 0.3
                    elide: Text.ElideRight
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
            activeFocusOnTab: root.pageStack.currentIndex == 0
            highlighted: focus
            onClicked: {
                if (systemsettings.activeCategory == index) {
                    root.pageStack.currentIndex = 1;
                } else {
                    systemsettings.activeCategory = index;
                    subCategoryColumn.title = model.display;
                }
            }
            onHoveredChanged: {
                if (hovered) {
                    systemsettings.requestToolTip(index, delegate.mapToItem(root, 0, 0, width, height));
                }
            }
            onFocusChanged: {
                if (focus) {
                    onCurrentIndexChanged: categoryView.positionViewAtIndex(index, ListView.Contain);
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
