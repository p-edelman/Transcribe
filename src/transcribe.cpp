#include "transcribe.h"

Transcribe::Transcribe(int &argc, char **argv) :
  QApplication(argc, argv) {

  m_text_file = NULL;

  // Initialize the audio player
  m_player = new AudioPlayer();
  QObject::connect(m_player, SIGNAL(audioError(const QString&)),
                   this, SLOT(errorDetected(const QString&)));

  QQmlApplicationEngine* engine = new QQmlApplicationEngine();

  // Expose the Transcribe object to the gui for setting and getting properties
  // and such
  engine->rootContext()->setContextProperty("app",    this);
  engine->rootContext()->setContextProperty("player", m_player);

  // This is a bit of quirkiness of Qt; you can't declare an enum as a QML type,
  // but you can declare a C++ with a public enum as a QML type, and than
  // access the enum values as type properties. So we expose the
  // "AudioPlayer" (the class) as "PlayerState" in QML.
  qmlRegisterType<AudioPlayer>("AudioPlayer", 1, 0, "PlayerState");

  // Load the GUI. When it is ready, the guiReady() method takes over.
  QObject::connect(engine, SIGNAL(objectCreated(QObject*, QUrl)),
                   this,   SLOT(guiReady(QObject*)));
  engine->load(QUrl(QStringLiteral("qrc:/main.qml")));
}

void Transcribe::setTextDirty(bool is_dirty) {
  if (m_is_text_dirty != is_dirty) {
    m_is_text_dirty = is_dirty;
    emit textDirtyChanged(is_dirty);
  }
}

bool Transcribe::isTextDirty() {
  return m_is_text_dirty;
}

QString Transcribe::getTextFileName() const {
  if (m_text_file != NULL) {
    QFileInfo info(m_text_file->fileName());
    return info.fileName();
  }
  return QString(tr("No transcript file loaded"));
}

bool Transcribe::saveText() {
  if (!m_text_file) {
    // This shouldn't happen because of the GUI. We silently ignore it
    qDebug() << "There is no text file loaded";
    return false;
  }

  // General error message for saving the file
  QString general_msg =  tr("There was an error saving the text file.\n");
  general_msg         += tr("The latest changes are not saved!");

  // We want the file saving process to be atomic or at least close, so we can't
  // loose any work. So we save the text to a temporary file first.
  QTemporaryFile temp_file;
  if (temp_file.open()) {
    QTextStream out_stream(&temp_file);
    out_stream << QQmlProperty::read(m_text_area, "text").toString();
    temp_file.close();
  } else {
    errorDetected(general_msg);
    qDebug() << "Couldn't create temp file";
    return false;
  }

  if (!m_text_file->isOpen()) {
    // Now move the existing file to a file with the .old extension, or .old1,
    // .old2, etc if it already exists
    QString old_file_name = NULL;
    if (m_text_file->exists()) {
      old_file_name = m_text_file->fileName() + ".old";
      unsigned int old_ext_counter = 1;
      while (QFile::exists(old_file_name)) {
        old_file_name + QString::number(old_ext_counter);
      }
      if (!QFile::rename(m_text_file->fileName(), old_file_name)) {
        errorDetected(general_msg);
        qDebug() << "Couldn't move file to .old file";
        return false;
      }
    }

    // Now we can move the temp file to our actual target file
    if (QFile::rename(temp_file.fileName(), m_text_file->fileName())) {
      // Indicate that the text is not dirty anymore
      setTextDirty(false);

      // Remove the .old file
      if (QFile::exists(old_file_name)) {
        QFile::remove(old_file_name);
      }
    } else {
      QString msg = tr("There was an error saving the text file.\n");
      if (old_file_name != NULL) {
        msg += tr("Your text file is saved as '") + old_file_name + "'\n";
      }
      msg += tr("Your recent edits are not saved!");
      errorDetected(msg);
      qDebug() << "Couldn't copy temp file " << temp_file.fileName() << " to " + m_text_file->fileName();
      return false;
    }
  } else {
    errorDetected(general_msg);
    qDebug() << "File is open";
    return false;
  }

  return true;
}

void Transcribe::errorDetected(const QString& message) {
  // Close any open modal windows. This is especially useful for an error with
  // audio loading, in which case the text file dialog is still active.
  QWidget* modal = activeModalWidget();
  while (modal != NULL) {
    modal->close();
    modal = activeModalWidget();
  }

  // Display the error box
  QMessageBox box;
  box.setText(message);
  box.setStandardButtons(QMessageBox::Ok);
  box.setIcon(QMessageBox::Critical);
  box.exec();
}

void Transcribe::close() {
  bool may_close = true;

  if (m_is_text_dirty) {
    QMessageBox box;
    box.setText(tr("The latest edits are not saved."));
    box.setInformativeText(tr("Do you want to save them now?"));
    box.setStandardButtons(QMessageBox::Save |
                           QMessageBox::Discard |
                           QMessageBox::Cancel );
    int action = box.exec();

    switch (action) {
      case QMessageBox::Save:
        may_close = saveText();
        break;
      case QMessageBox::Cancel:
        may_close = false;
        break;
    }
  }

  if (may_close) {
    quit();
  }
}

void Transcribe::guiReady(QObject* root) {
  m_app_root  = root;
  m_text_area = m_app_root->findChild<QObject *>("text_area");

  // Install the key filter
  KeyCatcher* catcher = new KeyCatcher(this, m_player, root);
  root->installEventFilter(catcher);

  // Attach audio controls to the AudioPlayer
  QObject* controls = root->findChild<QObject *>("media_controls");
  m_player->setAudioControls(controls);

  // Connect GUI events to their callbacks
  QObject::connect(m_app_root, SIGNAL(saveText()),
                   this, SLOT(saveText()));
  QObject::connect(m_app_root, SIGNAL(pickFiles()),
                   this, SLOT(pickFiles()));
  QObject::connect(m_app_root, SIGNAL(signalQuit()),
                   this, SLOT(close()));
}

void Transcribe::pickFiles() {
  // Let the user pick an audio file
  QString audio_file_path = QFileDialog::getOpenFileName(
    NULL, tr("Open an audio file"), NULL,
    tr("Audio files (*.wav *.mp3 *.aac *.amr *.aiff *.flac *.ogg *.wma)"));
  if (audio_file_path == NULL) return;

  // Unload the current text file
  m_text_file = NULL;
  emit textFileNameChanged();
  QQmlProperty::write(m_app_root, "is_editable", QVariant(false));

  // Open the audio file
  m_player->openAudioFile(audio_file_path);

  // Let the user pick a text file for the transcript. As a file suggestion, we
  // base a txt file on the current audio file.
  QFileInfo info(audio_file_path);
  QString suggestion = info.absolutePath() + "/" + info.baseName() + ".txt";
  QString text_file_path = QFileDialog::getSaveFileName(
    NULL, tr("Pick a text file for the transcript"),
    suggestion, tr("Text files (*.txt)"), NULL,
    QFileDialog::DontConfirmOverwrite);
  if (text_file_path == NULL) return;

  openTextFile(text_file_path);
}

void Transcribe::openTextFile(const QString& path) {
  // Because the way the UI works, we can assume that the text is not dirty
  // So if the file exists, we load the contents into the editor window.
  m_text_file = new QFile(path);
  if (m_text_file->exists()) {
    if (m_text_file->open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream in_stream(m_text_file);
      QVariant text(in_stream.readAll());
      QQmlProperty::write(m_text_area, "text", text);
      m_text_file->close();
      setTextDirty(false); // QML has signalled text is dirty because we changed
                           // text, so we need reset this.
    } else {
      QString msg = tr("The text file can't be read");
      errorDetected(msg);
      m_text_file = NULL;
      return;
    }
  } else {
    // If the text file doesn't exist, empty the editor and create the file
    // by saving the text to it
    QQmlProperty::write(m_text_area, "text", QVariant(""));
    if (!saveText()) return;
  }

  // Update the gui
  emit textFileNameChanged();
  QQmlProperty::write(m_app_root, "is_editable", QVariant(true));
  setTextDirty(false);
}
