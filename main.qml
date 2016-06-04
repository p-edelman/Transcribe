import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0

ApplicationWindow {
  id:    main_window
  title: qsTr("Transcriber")

  visible: true
  width:   640
  height:  480

  /** Signals that the user wants to save the text. */
  signal saveText()

  AudioFileChooser {
    id:         audio_file_chooser
    objectName: "audio_file_chooser"
  }

  TextFileChooser {
    id:             text_file_chooser
    objectName:     "text_file_chooser"
  }

  menuBar: MenuBar {
    Menu {
      title: qsTr("File")
      MenuItem {
        text:        qsTr("&Open audio")
        onTriggered: audio_file_chooser.open()
      }
      MenuItem {
        id:          save_text_menu_item
        text:        qsTr("&Save text")
        onTriggered: main_window.saveText()
        enabled:     false
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

      anchors.bottom: parent.bottom
      anchors.top:    parent.top
      anchors.left:   parent.left

      font.pixelSize: 12

      Component.onCompleted: setFileName("")
    }
    Text {
      id: dirty_display

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
    color: white;

    anchors.top:    media_controls.bottom
    anchors.right:  parent.right
    anchors.bottom: parent.bottom
    anchors.left:   parent.left

    TextEdit {
      id:         text_area
      objectName: "text_area"

      anchors.fill: parent

      focus:               true
      font.pixelSize:      12
      cursorVisible:       true
      textFormat:          Text.PlainText
      horizontalAlignment: Text.AlignLeft
      wrapMode:            TextEdit.WordWrap

      onTextChanged: setDirty(true)
    }
  }

  /** Set the name of the text file in the status bar, or indicate that there is
      no text file.
      @param file_name the name of the file, or an empty string to indicate
                       that there is no open file. */
  function setFileName(file_name) {
    if (file_name == "") {
      file_name_display.text        = qsTr("Unsaved")
      file_name_display.font.italic = true
    } else {
      file_name_display.text =        file_name
      file_name_display.font.italic = false
    }
  }

  /** Set the dirty status for the text file; thus the text in the text area
      is not saved. */
  function setDirty(is_dirty) {
    if (is_dirty) {
      dirty_display.text          = " *"
      save_text_menu_item.enabled = true
    } else {
      dirty_display.text = ""
      save_text_menu_item.enabled = false
    }
  }
}
