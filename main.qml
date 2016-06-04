import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0

ApplicationWindow {
  id:    main_window
  title: qsTr("Transcriber")

  visible: true
  width:   640
  height:  480

  /** Indicates whether the GUI is useable for editing text files. */
  property bool is_editable

  /** Signals that the user wants to save the text. */
  signal saveText()

  AudioFileChooser {
    id:         audio_file_chooser
    objectName: "audio_file_chooser"
  }

  TextFileChooser {
    id:         text_file_chooser
    objectName: "text_file_chooser"
  }

  menuBar: MenuBar {
    Menu {
      title: qsTr("File")
      MenuItem {
        text: {
          if (app.is_text_dirty) {
            qsTr("&Open audio (save text changes first)")
          } else {
            qsTr("&Open audio")
          }
        }
        enabled: {
          if (app.is_text_dirty) {false} else {true}
        }
        onTriggered: audio_file_chooser.open()
      }
      MenuItem {
        id:          save_text_menu_item
        text:        qsTr("&Save text")
        onTriggered: main_window.saveText()
        enabled: {
          if (app.is_text_dirty) {true} else {false}
        }
      }
      MenuItem {
        text:        qsTr("Exit")
        onTriggered: Qt.quit();
      }
    }
  }

  statusBar: StatusBar {
    Text {
      id: file_name_display

      text: app.text_file_name

      anchors.bottom: parent.bottom
      anchors.top:    parent.top
      anchors.left:   parent.left

      font.pixelSize: 12
    }
    Text {
      id: dirty_display

      text: {
        if (app.is_text_dirty) {" *"} else {""}
      }

      anchors.bottom: parent.bottom
      anchors.top:    parent.top
      anchors.left:   file_name_display.right

      font.pixelSize: 12
    }
  }

  MediaControls {
    id:         media_controls
    objectName: "media_controls"
  }

  Rectangle {
    color: {
      if (main_window.is_editable) {
        "white";
      } else {
        "lightgray"
      }
    }

    anchors.top:    media_controls.bottom
    anchors.right:  parent.right
    anchors.bottom: parent.bottom
    anchors.left:   parent.left

    TextEdit {
      id:         text_area
      objectName: "text_area"

      anchors.fill: parent

      readOnly:            !main_window.is_editable
      focus:               true
      font.pixelSize:      12
      cursorVisible:       true
      textFormat:          Text.PlainText
      horizontalAlignment: Text.AlignLeft
      wrapMode:            TextEdit.WordWrap

      onTextChanged: app.is_text_dirty = true
    }
  }

  Component.onCompleted: is_editable = false
}
