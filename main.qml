import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
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
    RowLayout {
      Text {
        id: file_name_display

        text: app.text_file_name

        font.pointSize: 12
      }
      Text {
        id: dirty_display

        text: {
          if (app.is_text_dirty) {" *"} else {""}
        }

        font.pointSize: 12
      }
    }
  }

  MediaControls {
    id:         media_controls
    objectName: "media_controls"
  }

  TextArea {
    id:         text_area
    objectName: "text_area"

    anchors.top:    media_controls.bottom
    anchors.right:  parent.right
    anchors.bottom: parent.bottom
    anchors.left:   parent.left

    readOnly:            !main_window.is_editable
    focus:               true
    font.pixelSize:      12
    textFormat:          Text.PlainText
    horizontalAlignment: Text.AlignLeft
    wrapMode:            TextEdit.WordWrap

    // Set the text to dirty status whenever it changes
    onTextChanged: app.is_text_dirty = true

    // On loading, the text gets changed so the status gets set to dirty, even
    // though the user didn't change anything. So we have to reset it after
    // loading.
    Component.onCompleted: app.is_text_dirty = false

    style: TextAreaStyle {
      backgroundColor: {
        if (main_window.is_editable) {
          "white";
        } else {
          "lightgray"
        }
      }
    }
  }

  Component.onCompleted: is_editable = false
}
