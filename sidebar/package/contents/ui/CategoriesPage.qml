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
import org.kde.kirigami 2.10 as Kirigami

Kirigami.ScrollablePage {
    id: mainColumn
    Component.onCompleted: searchField.forceActiveFocus()
    readonly property bool searchMode: searchField.text.length > 0

    header: Rectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        width: mainColumn.width
        height: Math.round(Kirigami.Units.gridUnit * 2.5)
        RowLayout {
            id: searchLayout
            spacing: Kirigami.Units.smallSpacing
            anchors {
                fill: parent
                margins: Kirigami.Units.smallSpacing
            }

            QQC2.ToolButton {
                id: menuButton
                icon.name: "application-menu"
                checkable: true
                checked: systemsettings.actionMenuVisible
                Keys.onBacktabPressed: {
                    root.focusPreviousRequest()
                }
                onClicked: systemsettings.showActionMenu(mapToGlobal(0, height))

                QQC2.ToolTip {
                    text: i18n("Show menu")
                }
            }

            Kirigami.SearchField {
                id: searchField
                focus: true
                Layout.fillWidth: true
                onTextChanged: {
                    systemsettings.searchModel.filterRegExp = text;
                }
                KeyNavigation.tab: categoryView
            }

            QQC2.ToolButton {
                id: showIntroPageButton
                enabled: !systemsettings.introPageVisible
                icon.name: "go-home"
                Keys.onBacktabPressed: {
                    root.focusPreviousRequest()
                }
                onClicked: systemsettings.introPageVisible = true

                QQC2.ToolTip {
                    text: i18n("Show intro page")
                }
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
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        color: Kirigami.Theme.backgroundColor
    }
    Kirigami.Heading {
        anchors.centerIn: parent
        width: parent.width * 0.7
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        text: i18nc("A search yielded no results", "No items matching your search")
        opacity: categoryView.count == 0 ? 0.3 : 0
        Behavior on opacity {
            OpacityAnimator {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    Binding {
        target: categoryView
        property: "currentIndex"
        value: mainColumn.searchMode
            ? systemsettings.activeSearchRow
            : systemsettings.activeCategoryRow
    }

    ListView {
        id: categoryView
        anchors.fill: parent
        model: mainColumn.searchMode ? systemsettings.searchModel : systemsettings.categoryModel

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
            delegate: Kirigami.ListSectionHeader {
                width: categoryView.width
                label: section
            }
        }

        delegate: Kirigami.AbstractListItem {
            id: delegate

            Accessible.role: Accessible.ListItem
            Accessible.name: model.display
            supportsMouseEvents: !model.IsCategoryRole || !mainColumn.searchMode
            enabled: !model.IsCategoryRole || !mainColumn.searchMode
            onClicked: {
                if (model.IsCategoryRole && mainColumn.searchMode) {
                    return;
                }

                if (mainColumn.searchMode || systemsettings.activeCategoryRow !== index) {
                    systemsettings.loadModule(categoryView.model.index(index, 0));
                }
                if (!mainColumn.searchMode && root.pageStack.depth > 1) {
                    root.pageStack.currentIndex = 1;
                }
            }
            onHoveredChanged: {
                if (model.IsCategoryRole && mainColumn.searchMode) {
                    return;
                }
                if (hovered) {
                    systemsettings.requestToolTip(categoryView.model.index(index, 0), delegate.mapToItem(root, 0, 0, width, height));
                } else {
                    systemsettings.hideToolTip();
                }
            }
            onFocusChanged: {
                if (model.IsCategoryRole && mainColumn.searchMode) {
                    return;
                }
                if (focus) {
                    categoryView.positionViewAtIndex(index, ListView.Contain);
                }
            }
            highlighted: categoryView.currentIndex == index
            Keys.onEnterPressed: clicked();
            Keys.onReturnPressed: clicked();
            contentItem: RowLayout {
                id: layout
                spacing: Kirigami.Settings.tabletMode ? Kirigami.Units.largeSpacing : Kirigami.Units.smallSpacing
                Kirigami.Icon {
                    id: icon
                    source: model.decoration
                    Layout.preferredHeight: Layout.preferredWidth
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                    Layout.leftMargin: (model.DepthRole-1) * (icon.width + layout.spacing) 
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    text: model.display
                    color: (delegate.highlighted || delegate.checked || (delegate.pressed && delegate.supportsMouseEvents)) ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                    elide: Text.ElideRight
                }
            }
        }
    }

    footer: Rectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        width: mainColumn.width
        height: Kirigami.Units.gridUnit * 2
        QQC2.ToolButton {
            anchors {
                fill: parent
                margins: Kirigami.Units.smallSpacing
            }
            text: i18nc("Action to show indicators for settings with custom data", "Highlight changed settings")
            icon.name: "tools"
            onClicked: systemsettings.toggleDefaultsIndicatorsVisibility()
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
