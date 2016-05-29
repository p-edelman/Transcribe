import QtQuick 2.2
import QtQuick.Dialogs 1.0

/** Chooser dialog for opening audio files.
    It emits the audioFileOpenendSignal() signal when a file is picked. */
FileDialog {
  title:          qsTr("Pick an audio file")
  selectExisting: true
  selectFolder:   false
  selectMultiple: false
  nameFilters:    ["Audio files (*.wav *.mp3)"]

  signal audioFileOpenendSignal(string file_url)

  onAccepted: {
    audioFileOpenendSignal(fileUrl)
  }
}
