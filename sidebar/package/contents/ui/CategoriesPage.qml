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
import org.kde.kirigami 2.14 as Kirigami
import org.kde.systemsettings 1.0

Kirigami.ScrollablePage {
    id: mainColumn
    Component.onCompleted: searchField.forceActiveFocus()
    readonly property bool searchMode: searchField.text.length > 0

    header: Kirigami.AbstractApplicationHeader {
        topPadding: Kirigami.Units.smallSpacing
        bottomPadding: Kirigami.Units.smallSpacing
        leftPadding: Kirigami.Units.smallSpacing
        rightPadding: Kirigami.Units.smallSpacing

        contentItem: RowLayout {
            anchors.fill: parent
            spacing: Kirigami.Units.smallSpacing

            QQC2.ToolButton {
                id: menuButton
                icon.name: "application-menu"
                checkable: true
                checked: systemsettings.actionMenuVisible
                Keys.onBacktabPressed: {
                    root.focusPreviousRequest()
                }
                onClicked: systemsettings.showActionMenu(mapToGlobal(0, height))

                Accessible.role: Accessible.Button
                Accessible.name: i18n("Show menu")
                QQC2.ToolTip {
                    text: parent.Accessible.name
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

                Accessible.role: Accessible.Button
                Accessible.name: i18n("Show intro page")
                QQC2.ToolTip {
                    text: parent.Accessible.name
                }
            }
        }
    }
    background: Rectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        color: Kirigami.Theme.backgroundColor
    }
    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.gridUnit * 4)
        text: i18nc("A search yielded no results", "No items matching your search")
        opacity: categoryView.count == 0 ? 1 : 0
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
            contentItem: CategoryItem {
                showArrow: model.IsCategoryRole
                selected: delegate.highlighted || delegate.pressed
                // Only indent subcategory icons in the search view
                isSearching: searchField.text.length > 0
            }
        }
    }

    footer: Rectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        width: mainColumn.width
        height: Kirigami.Units.gridUnit * 2
        visible: systemsettings.applicationMode == SystemSettings.SystemSettings
        QQC2.ToolButton {
            anchors {
                fill: parent
                margins: Kirigami.Units.smallSpacing
            }
            text: i18nc("Action to show indicators for settings with custom data", "Highlight Changed Settings")
            icon.name: "draw-highlight"
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
