import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0
import QtQml 2.2

ApplicationWindow {
  id:    main_window
  title: {
    var title = "Transcribe"
    if (Qt.platform.os != "android" && app.text_file_name != "") {
      title = app.text_file_name + " - " + title
      if (app.is_text_dirty) {
        title += " *"
      }
    }

    title
  }

  visible: true
  width:   640
  height:  480

  /** Indicates whether the GUI is useable for editing text files. */
  property bool is_editable

  /** Signals that the user wants to open a new audio and text file. */
  signal pickFiles()

  /** Signals that the user has selected an item from the history menu.
    * @param index the index in the history model of the item. */
  signal historySelected(int index)

  /** Signals that the user wants to quit the application. */
  signal signalQuit()

  /** Signals that the user wants to save the text. */
  signal saveText()

  /** Signals that the text should be shared with an external app (Android
      only). */
  signal shareText()

  /** Signals that the current text file should be deleted. */
  signal deleteText()

  /** Emitted when the user changes the position in the audio stream.
      @param seconds the time of the slider in seconds */
  signal audioPositionChanged(int seconds)

  /** Emitted when the playing status is changed by the user.
      @param is_playing indicates whether the user wants the audio to play. */
  signal playingStateChanged(bool is_playing)

  /** Emitted when the user presses a forward or rewind button.
      @param seconds the number of seconds to seek, either negative or positive.
    */
  signal seekAudio(int seconds)

  /** Emitted when the user wants to change the audio boosting.
      @param is_up whether the boost should be increased or decreased. */
  signal boostAudio(bool is_up)

  /** Emitted when the number of words might have been modified. */
  signal numWordsDirty()

  AndroidToolBar {
    id: android_toolbar
  }
  toolBar: android_toolbar

  menuBar: MenuBar {
    Menu {
      title: qsTr("&File")

      enabled: {
        if (stack.depth === 1) {true} else {false}
      }

      MenuItem {
        visible: Qt.platform.os != "android"
        text: {
          if (app.is_text_dirty) {
            qsTr("Open audio and text (save text changes first)")
          } else {
            qsTr("&Open audio and text")
          }
        }
        enabled:     app.is_text_dirty ? false : true
        onTriggered: pickFiles()
      }
      MenuItem {
        visible:     Qt.platform.os != "android"
        id:          save_text_menu_item
        text:        qsTr("&Save text")
        onTriggered: main_window.saveText()
        enabled:     app.is_text_dirty ? true : false
      }
      MenuItem {
        // Android only
        visible:     Qt.platform.os == "android"
        id:          android_delete_menu_item
        text:        qsTr("Delete transcription")
        enabled:     app.text_file_name == "" ? false : true
        onTriggered: main_window.deleteText()
      }
      Menu {
        id: history_submenu
        title: qsTr("History")

        // Instantiate the items in the history menu from the HistoryModel,
        // which is exposed as 'history'
        Instantiator {
          model: history
          delegate: MenuItem {
            text: display
            onTriggered: historySelected(index)
          }
          onObjectAdded:   history_submenu.insertItem(index, object)
          onObjectRemoved: history_submenu.removeItem(object)
        }
      }

      MenuItem {
        id:          settings_menu_item
        text:        qsTr("Settings")
        onTriggered: {
          playingStateChanged(false)
          stack.push(config_window)
        }
      }

      MenuItem {
        text:        qsTr("Exit")
        onTriggered: signalQuit()
      }
    }
  }

  statusBar: StatusBar {
    RowLayout { // We use a single to make the status bar auto scale to its contents
      anchors.fill: parent
      spacing:      0

      Text {
        id:   file_name_display
        text: app.text_file_name != "" ? app.text_file_name : qsTr("No transcript file loaded")
      }
      Text {
        id: dirty_display
        text: app.is_text_dirty ? " *" : ""
      }
      Item {
        Layout.fillWidth: true // Right align the word count
      }
      Text {
        id:   words_num_display
        text: app.num_words
      }
      Text {
        id:   words_text_display
        text: " words"
      }
    }
  }

  // To allow for mobile platforms, we use a StackView to display the settings
  // window and other temporary sub-windows in the main window.
  StackView {
    id:           stack
    anchors.fill: parent
    initialItem:  main_area
  }

  // The actual app GUI
  MainArea {
    id: main_area
  }

  SettingsGUI {
    id:      config_window
    visible: false
    onSettingsDone: stack.pop()
  }

  onClosing: {
    // Don't just close the window yet
    close.accepted = false

    if ((Qt.platform.os == "android") && (stack.depth > 1)) {
      // We caught the Android back button and just need to navigate back on the
      // stack
      stack.pop()
    } else {
      // Let the Transcribe app decide if we want to quit
      signalQuit()
    }
  }

  Component.onCompleted: {
    is_editable = false

    // Connect the signals from the audio player to the main interface signals
    main_area.valueChanged.connect(audioPositionChanged)
    main_area.playingStateChanged.connect(playingStateChanged)
    main_area.seekAudio.connect(seekAudio)
    main_area.boostAudio.connect(boostAudio)
    main_area.numWordsDirty.connect(numWordsDirty)
  }
}
