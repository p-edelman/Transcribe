import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0

// The toolbar provides the 'quick access' buttons on Android, for which
// especially the save button is important. For other platforms it would
// just take up space, so we hide it there.
ToolBar {
  visible: Qt.platform.os == "android"

  RowLayout {
    anchors.fill: parent
    spacing: 0

    Item {
      Layout.fillWidth: true // Right align everything
    }
    ToolButton {
      Image {
        source: "image://translatedicon/open" + (enabled ? "" : "/inactive")
        sourceSize.width:  parent.height * 0.75
        sourceSize.height: parent.height * 0.75
        anchors.centerIn: parent
      }
      enabled:   app.is_text_dirty ? false : true
      onClicked: pickFiles()
    }
    ToolButton {
      Image {
        source: "image://translatedicon/save" + (enabled ? "" : "/inactive")
        sourceSize.width:  parent.height * 0.75
        sourceSize.height: parent.height * 0.75
        anchors.centerIn: parent
      }
      enabled:   app.is_text_dirty ? true : false
      onClicked: main_window.saveText()
    }
    ToolButton {
      Image {
        source: "image://translatedicon/send" + (enabled ? "" : "/inactive")
        sourceSize.width:  parent.height * 0.75
        sourceSize.height: parent.height * 0.75
        anchors.centerIn: parent
      }
      enabled:   app.text_file_name == "" ? false : true
      onClicked: main_window.shareText()
    }
  }
}
