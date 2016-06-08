import QtQuick 2.2
import QtQuick.Dialogs 1.0

/** Chooser dialog for opening and saving transcript text files.
    It emits the textFileChosenSignal() signal when a file is picked. */
FileDialog {
  title:          qsTr("Open or create a transcript text file")
  selectFolder:   false
  selectMultiple: false
  selectExisting: false
  nameFilters:    ["Text files (*.txt)", "All (*.*)"]

  signal textFileChosenSignal(url file_url)

  onAccepted: {
    console.log("Chose file " + fileUrl)
    textFileChosenSignal(fileUrl)
  }
}
