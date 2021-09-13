/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.15
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.14 as Kirigami
import org.kde.systemsettings 1.0

Kirigami.ScrollablePage {
    id: mainColumn
    Component.onCompleted: searchField.forceActiveFocus()
    readonly property bool searchMode: searchField.text.length > 0

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

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

    GridLayout {
        id: categoryView
        anchors.fill: parent

        columns: Math.floor(mainColumn.width / 160)

        Repeater {
            id: categoryRepeater
            model: systemsettings.categoryModel

            delegate: ColumnLayout {
                Accessible.name: model.display

                Kirigami.Icon {
                    source: model.decoration
                    Layout.alignment: Qt.AlignHCenter
                }
                QQC2.Label {
                    text: model.display
                    horizontalAlignment: Qt.AlignHCenter

                    Layout.preferredWidth: 150
                }

                TapHandler {
                    onTapped: {
                        if (!model.IsKCMRole && mainColumn.searchMode) {
                            return;
                        }

                        if (model.IsKCMRole || mainColumn.searchMode || systemsettings.activeCategoryRow !== index) {
                            systemsettings.loadModule(categoryRepeater.model.index(index, 0));
                        }
                        if (!mainColumn.searchMode && root.pageStack.depth > 1) {
                            root.pageStack.currentIndex = 1;
                        }
                    }
                }
            }
        }
    }

    // ListView {
    //     id: categoryView
    //     anchors.fill: parent
    //     model: mainColumn.searchMode ? systemsettings.searchModel : systemsettings.categoryModel

    //     onContentYChanged: systemsettings.hideToolTip();
    //     activeFocusOnTab: true
    //     keyNavigationWraps: true
    //     Accessible.role: Accessible.List
    //     Keys.onTabPressed: {
    //         if (applicationWindow().wideScreen) {
    //             subCategoryColumn.focus = true;
    //         } else {
    //             root.focusNextRequest();
    //         }
    //     }
    //     section {
    //         property: "categoryDisplayRole"
    //         delegate: Kirigami.ListSectionHeader {
    //             width: categoryView.width
    //             label: section
    //         }
    //     }

    //     delegate: CategoryItem {
    //         id: delegate

    //         showArrow: {
    //             if (!model.IsCategoryRole) {
    //                 return false;
    //             }
    //             const modelIndex = delegate.ListView.view.model.index(index, 0)
    //             return delegate.ListView.view.model.rowCount(modelIndex) > 1
    //         }
    //         // Only indent subcategory icons in the search view
    //         leadingPadding: (model.DepthRole > 1 && searchField.text.length > 0) ? (( model.DepthRole - 1 ) * Kirigami.Units.iconSizes.smallMedium) + Kirigami.Units.largeSpacing : 0

    //         hoverEnabled: !model.IsCategoryRole || !mainColumn.searchMode
    //         enabled: model.IsKCMRole || !mainColumn.searchMode
    //         onHoveredChanged: {
    //             if (model.IsCategoryRole && mainColumn.searchMode) {
    //                 return;
    //             }
    //             if (hovered) {
    //                 systemsettings.requestToolTip(categoryView.model.index(index, 0), delegate.mapToItem(root, 0, 0, width, height));
    //             } else {
    //                 systemsettings.hideToolTip();
    //             }
    //         }
    //         onFocusChanged: {
    //             if (model.IsCategoryRole && mainColumn.searchMode) {
    //                 return;
    //             }
    //             if (focus) {
    //                 categoryView.positionViewAtIndex(index, ListView.Contain);
    //             }
    //         }
    //         Keys.onEnterPressed: clicked();
    //         Keys.onReturnPressed: clicked();
    //     }
    // }
}
