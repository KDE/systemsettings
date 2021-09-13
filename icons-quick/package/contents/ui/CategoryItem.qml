/*
 *   SPDX-FileCopyrightText: 2021 Jan Blackquill <uhhadd@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */
import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.13 as Kirigami

Kirigami.BasicListItem {
    id: delegate

    property bool showArrow: false
    property bool selected: delegate.highlighted || delegate.pressed
    property bool isSearching: false

    // Dummy item to make leadingPadding value manipulable by clients
    leading: Item {
        width: 0
    }
    leadingPadding: 0

    icon: model.decoration
    text: model.display
    Accessible.name: model.display

    trailing: RowLayout {
        Rectangle {
            id: defaultIndicator
            radius: width * 0.5
            implicitWidth: Kirigami.Units.largeSpacing
            implicitHeight: Kirigami.Units.largeSpacing
            visible: model.showDefaultIndicator && systemsettings.defaultsIndicatorsVisible
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            color: Kirigami.Theme.neutralTextColor
        }

        // Extra space to make the defaults indicators line up vertically for all items
        Item {
            visible: defaultIndicator.visible && !arrow.visible
            implicitWidth: arrow.Layout.preferredWidth
        }

        Kirigami.Icon {
            id: arrow
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: Kirigami.Units.iconSizes.small
            opacity: 0.7
            Layout.preferredWidth: Layout.preferredHeight
            source: (LayoutMirroring.enabled ? "go-next-symbolic-rtl" : "go-next-symbolic")
            visible: delegate.showArrow
            selected: delegate.selected
        }
    }
}
