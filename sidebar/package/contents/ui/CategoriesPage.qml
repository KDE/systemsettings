/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
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
        id: pageHeader

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
            // FIXME: left and right anchors shouldn't be needed here, but if
            // they're removed, the layout doesn't span the full width
            anchors {
                left: parent.left
                right: parent.right
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
        Kirigami.Theme.inherit: false
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

        delegate: CategoryItem {
            id: delegate

            showArrow: {
                if (!model.IsCategoryRole) {
                    return false;
                }
                const modelIndex = delegate.ListView.view.model.index(index, 0)
                return delegate.ListView.view.model.rowCount(modelIndex) > 1
            }
            // Only indent subcategory icons in the search view
            leadingPadding: (model.DepthRole > 1 && searchField.text.length > 0) ? (( model.DepthRole - 1 ) * Kirigami.Units.iconSizes.smallMedium) + Kirigami.Units.largeSpacing : 0

            hoverEnabled: !model.IsCategoryRole || !mainColumn.searchMode
            enabled: model.IsKCMRole || !mainColumn.searchMode

            onClicked: {
                if (!model.IsKCMRole && mainColumn.searchMode) {
                    return;
                }

                if (model.IsKCMRole || mainColumn.searchMode || systemsettings.activeCategoryRow !== index) {
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
            Keys.onEnterPressed: clicked();
            Keys.onReturnPressed: clicked();
        }
    }

    footer: QQC2.ToolBar {
        visible: systemsettings.applicationMode == SystemSettings.SystemSettings

        QQC2.ToolButton {
            anchors.fill: parent

            text: i18nc("Action to show indicators for settings with custom data. Use as short a translation as is feasible, as horizontal space is limited.", "Highlight Changed Settings")
            icon.name: "draw-highlight"

            onToggled: systemsettings.toggleDefaultsIndicatorsVisibility()
            checkable: true
            checked: systemsettings.defaultsIndicatorsVisible
        }
    }
}
