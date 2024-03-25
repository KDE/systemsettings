/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.15
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.5 as Kirigami

import org.kde.systemsettings

Kirigami.ScrollablePage {
    id: subCategoryColumn
    title: systemsettings.subCategoryModel.title

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

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
            textFormat: Text.PlainText
        }

        contentItem: RowLayout {
            id: rowLayout
            // FIXME: left and right anchors shouldn't be needed here, but if
            // they're removed, the layout doesn't span the full width
            anchors.fill: parent
            spacing: Math.round(Kirigami.Units.smallSpacing/2) // Match margins

            Keys.onDownPressed: event => {
                subCategoryView.currentIndex = 0;
                event.accepted = false; // Pass to KeyNavigation.down
            }

            // Show a back Button when only one column is visible
            QQC2.ToolButton {
                id: backButton

                Layout.fillWidth: true
                visible: !applicationWindow().wideScreen
                // Uncomment once QQC2.ToolButton can force-left-align its contents
                // text: subCategoryColumn.title
                // icon.name: LayoutMirroring.enabled ? "go-previous-symbolic-rtl" : "go-previous-symbolic"
                onClicked: { root.pageStack.currentIndex = 0; }

                // Need a custom content item to left-align everything, because
                // ToolButtons center everything when you force the width to be
                // higher than normal, which looks bad here
                contentItem: RowLayout {
                    Kirigami.Icon {
                        id: backIcon
                        source: LayoutMirroring.enabled ? "go-previous-symbolic-rtl" : "go-previous-symbolic"
                        Layout.leftMargin: Kirigami.Units.smallSpacing
                        Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                        Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                        Layout.alignment: Qt.AlignCenter
                    }
                    Kirigami.Heading {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        level: 4
                        text: subCategoryColumn.title
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        textFormat: Text.PlainText
                    }
                }

                KeyNavigation.right: hamburgerMenuButton
                KeyNavigation.down: subCategoryView
                KeyNavigation.tab: KeyNavigation.right
                Keys.onBacktabPressed: {
                    systemsettings.focusPrevious()
                }
                Keys.onDownPressed: event => rowLayout.Keys.downPressed(event)
            }

            // Show a non-interactive heading when both columns are visible
            Kirigami.Heading {
                visible: !backButton.visible
                Layout.fillWidth: true
                Layout.fillHeight: true
                // Don't be too short when the back and burger buttons aren't visible
                Layout.minimumHeight: Math.max(backButton.implicitHeight, hamburgerMenuButton.implicitHeight)
                Layout.leftMargin: backIcon.visible ? 0 : Kirigami.Units.smallSpacing
                level: 3
                text: subCategoryColumn.title
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                textFormat: Text.PlainText
            }

            HamburgerMenuButton {
                id: hamburgerMenuButton

                visible: !applicationWindow().wideScreen

                KeyNavigation.left: backButton
                KeyNavigation.down: subCategoryView
                KeyNavigation.backtab: KeyNavigation.left
                KeyNavigation.tab: KeyNavigation.down

                Keys.onDownPressed: event => rowLayout.Keys.downPressed(event)
            }
        }
    }

    ListView {
        id: subCategoryView

        anchors.fill: parent
        model: systemsettings.subCategoryModel
        currentIndex: systemsettings.activeSubCategoryRow
        activeFocusOnTab: true
        keyNavigationWraps: true
        Accessible.role: Accessible.List

        KeyNavigation.up: backButton
        KeyNavigation.backtab: hamburgerMenuButton

        Keys.onUpPressed: event => {
            if (subCategoryView.currentIndex === 0) {
                subCategoryView.currentIndex = -1;
            }
            event.accepted = false; // Pass to KeyNavigation.up
        }
        Keys.onTabPressed: systemsettings.focusNext();

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
            function onSearchModeChanged() {
                if (root.searchMode) {
                    root.pageStack.pop(mainColumn);
                } else if (subCategoryView.count > 1) {
                    root.pageStack.push(subCategoryColumn);
                }
            }
        }
        Connections {
            target: systemsettings
            function onActiveSubCategoryRowChanged() {
                subCategoryView.currentIndex = systemsettings.activeSubCategoryRow
                if (systemsettings.activeSubCategoryRow >= 0) {
                    if (subCategoryView.count > 1) {
                        root.pageStack.push(subCategoryColumn);
                    }
                    root.pageStack.currentIndex = 1;
                    subCategoryView.forceActiveFocus();
                }
            }
            function onIntroPageVisibleChanged() {
                if (systemsettings.introPageVisible) {
                    root.pageStack.pop(mainColumn)
                }
            }
        }

        delegate: CategoryItem {
            id: delegate

            // Indent items that are children of other KCMs within the same group
            leadingPadding: model.DepthRole > 2 ? (( model.DepthRole - 2 ) * Kirigami.Units.iconSizes.smallMedium) + Kirigami.Units.largeSpacing : 0

            highlighted: ListView.isCurrentItem

            onClicked: {
                systemsettings.loadModule(subCategoryView.model.index(index, 0));
            }
            onFocusChanged: {
                if (focus) {
                    onCurrentIndexChanged: subCategoryView.positionViewAtIndex(index, ListView.Contain);
                }
            }
            Keys.onEnterPressed: clicked();
            Keys.onReturnPressed: clicked();

            Keys.onEscapePressed: root.pageStack.currentIndex = 0;

            Keys.onLeftPressed: {
                if (LayoutMirroring.enabled) {
                    clicked();
                } else {
                    root.pageStack.currentIndex = 0;
                }
            }
            Keys.onRightPressed: {
                if (!LayoutMirroring.enabled) {
                    clicked();
                } else {
                    root.pageStack.currentIndex = 0;
                }
            }
        }
    }
}
