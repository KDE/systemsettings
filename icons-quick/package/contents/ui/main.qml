/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.5

import org.kde.kirigami 2.5 as Kirigami

Kirigami.ApplicationItem {
    id: root
    implicitWidth: wideScreen ? Kirigami.Units.gridUnit * 30 :  Kirigami.Units.gridUnit * 15
    pageStack.initialPage: mainColumn
    pageStack.defaultColumnWidth: wideScreen ? root.width / 2 : root.width

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    property alias searchMode: mainColumn.searchMode
    readonly property real headerHeight: mainColumn.header.height

    signal focusNextRequest()
    signal focusPreviousRequest()

    function focusFirstChild() {
        mainColumn.focus = true;
    }

    function focusLastChild() {
        subCategoryColumn.focus = true;
    }

    wideScreen: pageStack.depth > 1 && systemsettings.width > Kirigami.Units.gridUnit * 70
    CategoriesPage {
        id: mainColumn
        focus: true
    }

    SubCategoryPage {
        id: subCategoryColumn
        KeyNavigation.left: mainColumn
    }
    Kirigami.Separator {
        z: 999
        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }
    }
}
