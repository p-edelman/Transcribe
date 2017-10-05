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

    Item {
      Layout.fillWidth: true // Right align everything
    }
    ToolButton {
      iconSource: enabled ? "image://materialicon/open" : "image://materialicon/open/inactive"
      enabled:   app.is_text_dirty ? false : true
      onClicked: pickFiles()
    }
    ToolButton {
      iconSource: enabled ? "image://materialicon/save" : "image://materialicon/save/inactive"
      enabled:   app.is_text_dirty ? true : false
      onClicked: main_window.saveText()
    }
    ToolButton {
      iconSource: enabled ? "image://materialicon/send" : "image://materialicon/send/inactive"
      enabled:   app.text_file_name == "" ? false : true
      onClicked: main_window.shareText()
    }
  }
}
