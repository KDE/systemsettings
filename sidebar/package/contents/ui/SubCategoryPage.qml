/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.15
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.5 as Kirigami
import org.kde.systemsettings 1.0

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
        }

         background: Rectangle {
            color: Kirigami.Theme.highlightColor
            opacity: hoverHandler.hovered ? (tapHandler.pressed ? 0.4 : 0.2) : 0
            visible: !applicationWindow().wideScreen
            Accessible.role: Accessible.Button
            Accessible.name: i18n("Go back")
            TapHandler {
                id: tapHandler
                acceptedButtons: applicationWindow().wideScreen ? Qt.NoButton : Qt.LeftButton
                onTapped: root.pageStack.currentIndex = 0;
            }
            HoverHandler {
                id: hoverHandler
                enabled: !Kirigami.Settings.isMobile
            }
            QQC2.ToolTip {
                text: parent.Accessible.name
            }
        }

        contentItem: RowLayout {
            id: toolBarLayout
            anchors.fill: parent
            spacing: 0

            // NOTE: this toolbutton is needed even if is not visible in order to have good
            // size hints for the toolbar height which should perfectly align with the toolbar on the other side
            QQC2.ToolButton {
                id: buttonSizePlaceholder
                visible: false
                icon.name: LayoutMirroring.enabled ? "go-previous-symbolic-rtl" : "go-previous-symbolic"
            }
            Kirigami.Icon {
                id: backIcon
                visible: !applicationWindow().wideScreen
                source: LayoutMirroring.enabled ? "go-previous-symbolic-rtl" : "go-previous-symbolic"
                Layout.preferredWidth: buttonSizePlaceholder.implicitWidth
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                Layout.alignment: Qt.AlignCenter
            }

            Kirigami.Heading {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: backIcon.visible ? 0 : Kirigami.Units.smallSpacing
                // Don't be too short when the back button isn't visible
                Layout.minimumHeight: buttonSizePlaceholder.implicitHeight
                level: 3
                text: subCategoryColumn.title
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

        }
    }

    ListView {
        id: subCategoryView
        anchors.fill: parent
        model: systemsettings.subCategoryModel
        currentIndex: systemsettings.activeSubCategoryRow
        onContentYChanged: systemsettings.hideToolTip();
        activeFocusOnTab: true
        keyNavigationWraps: true
        Accessible.role: Accessible.List
        Keys.onTabPressed: root.focusNextRequest();
        Keys.onBacktabPressed: {
            mainColumn.focus = true;
        }

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

            onClicked: {
                systemsettings.loadModule(subCategoryView.model.index(index, 0));
            }
            onHoveredChanged: {
                if (hovered) {
                    systemsettings.requestToolTip(subCategoryView.model.index(index, 0), delegate.mapToItem(root, 0, 0, width, height));
                } else {
                    systemsettings.hideToolTip();
                }
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

    footer: FooterToolbar {
        visible: systemsettings.applicationMode == SystemSettings.SystemSettings &&
                     !applicationWindow().wideScreen
    }
}
