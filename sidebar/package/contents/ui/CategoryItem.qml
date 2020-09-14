import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.10 as Kirigami

RowLayout {
    id: layout

    property bool showArrow: false
    property bool selected: false

    spacing: Kirigami.Settings.tabletMode ? Kirigami.Units.largeSpacing : Kirigami.Units.smallSpacing
    Kirigami.Icon {
        id: icon
        source: model.decoration
        Layout.preferredHeight: Layout.preferredWidth
        Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
        Layout.leftMargin: model.DepthRole > 1 ? (model.DepthRole - 1) * (Kirigami.Units.iconSizes.smallMedium + layout.spacing) : 0
    }
    QQC2.Label {
        Layout.fillWidth: true
        text: model.display
        color: (delegate.highlighted || delegate.checked || (delegate.pressed && delegate.supportsMouseEvents)) ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
        elide: Text.ElideRight
    }

    Kirigami.Icon {
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: Kirigami.Units.iconSizes.small
        // This is to vertically align the defaults indicators when visible
        Layout.rightMargin: systemsettings.defaultsIndicatorsVisible && !defaultIndicator.visible ? defaultIndicator.implicitWidth + layout.spacing : 0
        opacity: 0.7
        Layout.preferredWidth: Layout.preferredHeight
        source: (LayoutMirroring.enabled ? "go-next-symbolic-rtl" : "go-next-symbolic")
        visible: layout.showArrow
        selected: layout.selected
    }

    Rectangle {
        id: defaultIndicator
        radius: width * 0.5
        implicitWidth: Kirigami.Units.largeSpacing
        implicitHeight: Kirigami.Units.largeSpacing
        visible: model.showDefaultIndicator && systemsettings.defaultsIndicatorsVisible
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        color: Kirigami.Theme.neutralTextColor
    }
}
