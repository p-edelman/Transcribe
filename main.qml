import QtQuick 2.4
import QtQuick.Controls 1.4

ApplicationWindow {
  visible: true
  width:   640
  height:  480
  title:   qsTr("Transcriber")

  AudioFileChooser {
    id:         audio_file_chooser
    objectName: "audio_file_chooser"
  }

  menuBar: MenuBar {
    Menu {
      title: qsTr("File")
      MenuItem {
        text: qsTr("&Open audio")
        onTriggered: audio_file_chooser.open()
      }
      MenuItem {
        text: qsTr("Exit")
        onTriggered: Qt.quit();
      }
    }
  }

  MediaControls {
    objectName: "media_controls"
  }
}
