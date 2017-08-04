import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.0

/** A 'toolbar' button for Android, providing acces to many Material style icons
    using the icon font from https://material.io. */
ToolButton {
  FontLoader {
    id:     materialIcons
    source: "qrc:///fonts/MaterialIcons.ttf"
  }

  style: ButtonStyle {
    background: Rectangle {
      color: "transparent"
    }

    label: Text {
      font.family:    materialIcons.name
      font.pointSize: 22
      color:          control.enabled ? "black" : "grey"
      text:           control.text
    }
  }
}
