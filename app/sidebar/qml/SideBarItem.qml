/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
   SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.15

import org.kde.kirigami 2.20 as Kirigami

Kirigami.ApplicationItem {
    id: root

    implicitWidth: wideScreen ? Kirigami.Units.gridUnit * 30 :  Kirigami.Units.gridUnit * 15
    wideScreen: pageStack.depth > 1 && systemsettings.width > Kirigami.Units.gridUnit * 70

    pageStack.initialPage: mainColumn
    pageStack.defaultColumnWidth: wideScreen ? root.width / 2 : root.width

    property alias searchMode: mainColumn.searchMode
    readonly property real headerHeight: mainColumn.header.height

    CategoriesPage {
        id: mainColumn
        focus: true
    }

    SubCategoryPage {
        id: subCategoryColumn
    }
}
