import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.0
import QtQml 2.2

ApplicationWindow {
  id:    main_window
  title: qsTr("Transcriber")

  visible: true
  width:   640
  height:  480

  /** Indicates whether the GUI is useable for editing text files. */
  property bool is_editable

  /** Signals that the user wants to open a new audio and text file. */
  signal pickFiles()

  /** Signals that the user wants to quit the application. */
  signal signalQuit()

  /** Signals that the user wants to save the text. */
  signal saveText()

  menuBar: MenuBar {
    Menu {
      title: qsTr("&File")
      MenuItem {
        text: {
          if (app.is_text_dirty) {
            qsTr("Open audio and text (save text changes first)")
          } else {
            qsTr("&Open audio and text")
          }
        }
        enabled: {
          if (app.is_text_dirty) {false} else {true}
        }
        onTriggered: pickFiles()
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
        onTriggered: signalQuit()
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

    // Text input seems to be somewhat borked at the moment on touch based
    // platforms (or at least Android). Touch is registered as a mouse click,
    // so the user can't drag/flick long text but ends up selecting it.
    // To prevent this, we can set the ability to select by mouse to false on
    // touch based platforms. This is of course a crude tool; it's now not
    // possible to use an attached mouse to select text. Worse, it's also
    // not possible to select text the proper touch based way, with a long
    // press, only the keyboard will work.
    selectByMouse: {
      switch (Qt.platform.os) {
        case "android":
        case "blackberry":
        case "ios":
        case "winphone":
          false
          break
        default:
          true
      }
    }

    readOnly:            !main_window.is_editable
    focus:               true
    font.pointSize:      12
    textFormat:          Text.PlainText
    horizontalAlignment: Text.AlignLeft
    wrapMode:            TextEdit.WordWrap
    flickableItem {
      flickableDirection: Flickable.VerticalFlick
    }

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

  onClosing: {
    // Don't just close the window, but let the Transcribe app decide.
    close.accepted = false
    signalQuit()
  }

  Component.onCompleted: is_editable = false
}
