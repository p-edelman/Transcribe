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
    id:         media_controls
    objectName: "media_controls"
  }

  Rectangle {
    color: white;

    anchors.top:    media_controls.bottom
    anchors.right:  parent.right
    anchors.bottom: parent.bottom
    anchors.left:   parent.left

    TextEdit {
      id: text_area

      anchors.fill: parent

      focus:               true
      font.pixelSize:      12
      cursorVisible:       true
      textFormat:          Text.PlainText
      horizontalAlignment: Text.AlignLeft
      wrapMode:            TextEdit.WordWrap
    }
  }

}
