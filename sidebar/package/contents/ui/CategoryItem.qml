import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.10 as Kirigami

RowLayout {
    id: layout
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
