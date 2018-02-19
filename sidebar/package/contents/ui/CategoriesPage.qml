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

    header: Rectangle {
        color: Kirigami.Theme.backgroundColor
        width: mainColumn.width
        height: Kirigami.Units.gridUnit * 2.5
        RowLayout {
            id: searchLayout
            spacing: Kirigami.Units.smallSpacing
            anchors {
                fill: parent
                margins: Kirigami.Units.smallSpacing
            }
            QtControls.ToolButton {
                id: menuButton
                iconName: "application-menu"
                Layout.maximumWidth: Kirigami.Units.iconSizes.smallMedium + Kirigami.Units.smallSpacing * 2
                Layout.maximumHeight: width
                Keys.onBacktabPressed: {
                    root.focusPreviousRequest()
                }
                menu: ActionMenu {
                    actions: ["configure", "help_contents", "help_about_app", "help_about_kde"]
                }
            }
            QtControls.TextField {
                id: searchField
                focus: true
                Layout.minimumHeight: Layout.maximumHeight
                Layout.maximumHeight: Kirigami.Units.iconSizes.smallMedium + Kirigami.Units.smallSpacing * 2
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
            visible: !categoryView.atYBeginning
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
        activeFocusOnTab: true
        keyNavigationWraps: true
        Accessible.role: Accessible.List
        Keys.onTabPressed: {
            if (applicationWindow().wideScreen) {
                subCategoryColumn.focus = true;
            } else {
                root.focusNextRequest();
            }
        }
        section {
            property: "categoryDisplayRole"
            delegate: Kirigami.AbstractListItem {
                separatorVisible: false
                supportsMouseEvents: false
                RowLayout {
                    anchors {
                        left: parent.left
                        right: parent.right
                        leftMargin: Kirigami.Units.smallSpacing
                    }
                    QtControls2.Label {
                        id: sectionLabel
                        text: section
                        Layout.minimumHeight: Math.max(implicitHeight, Kirigami.Units.iconSizes.smallMedium)
                        elide: Text.ElideRight
                        //FIXME: kirigami bug, why?
                        Component.onCompleted: font.bold = true
                    }
                }
            }
        }

        delegate: Kirigami.BasicListItem {
            id: delegate
            icon: model.decoration
            label: model.display
            separatorVisible: false
            highlighted: focus
            Accessible.role: Accessible.ListItem
            Accessible.name: model.display
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
                } else {
                    systemsettings.hideToolTip();
                }
            }
            onFocusChanged: {
                if (focus) {
                    onCurrentIndexChanged: categoryView.positionViewAtIndex(index, ListView.Contain);
                }
            }
            checked: systemsettings.activeCategory == index
        }
    }
}
