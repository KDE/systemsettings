import QtQuick 2.15
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.14 as Kirigami
import org.kde.systemsettings 1.0

Kirigami.AbstractApplicationHeader {
    id: pageHeader

    leftPadding: Kirigami.Units.smallSpacing
    rightPadding: Kirigami.Units.smallSpacing
    topPadding: Kirigami.Units.largeSpacing
    bottomPadding: Kirigami.Units.largeSpacing

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
    }
}