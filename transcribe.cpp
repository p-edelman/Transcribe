#include "transcribe.h"

Transcribe::Transcribe(int& argc, char** argv) : QGuiApplication(argc, argv) {
  m_player    = new AudioPlayer();
  m_text_file = NULL;
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

void Transcribe::saveText() {
  if (!m_text_file) {
    // This shouldn't happen because of the GUI. We silently ignore it
    qDebug() << "There is no text file loaded";
    return;
  }

  // We want the file saving process to be atomic or at least close, so we can't
  // loose any work. So we save the text to a temporary file first.
  QTemporaryFile temp_file;
  if (temp_file.open()) {
    QTextStream out_stream(&temp_file);
    out_stream << QQmlProperty::read(m_text_area, "text").toString();
    temp_file.close();
  } else {
    // TODO
    qDebug() << "Couldn't create temp file";
  }

  if (!m_text_file->isOpen()) {
    // Now move the existing file to a file with the .old extension, or .old1,
    // .old2, etc if it already exists
    QString old_file_name;
    if (m_text_file->exists()) {
      old_file_name = m_text_file->fileName() + ".old";
      unsigned int old_ext_counter = 1;
      while (QFile::exists(old_file_name)) {
        qDebug() << "Searching for old file name";
        old_file_name + QString::number(old_ext_counter);
      }
      if (!QFile::rename(m_text_file->fileName(), old_file_name)) {
        // TODO
        qDebug() << "Couldn't move file to .old file";
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
      // TODO
      qDebug() << "Couldn't copy temp file " << temp_file.fileName() << " to " + m_text_file->fileName();
    }
  } else {
    // TODO
    qDebug() << "File is open";
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
  QObject* audio_chooser = root->findChild<QObject*>("audio_file_chooser");
  QObject::connect(audio_chooser, SIGNAL(audioFileOpenendSignal(QString)),
                   this, SLOT(audioFilePicked(QString)));
  QObject* text_file_chooser = root->findChild<QObject*>("text_file_chooser");
  QObject::connect(text_file_chooser, SIGNAL(textFileChosenSignal(QString)),
                   this, SLOT(textFilePicked(QString)));
}

void Transcribe::audioFilePicked(const QString &url) {
  // Unload the current text file
  m_text_file = NULL;
  emit textFileNameChanged();
  QQmlProperty::write(m_app_root, "is_editable", QVariant(false));

  // Open the audio file
  m_player->openAudioFile(url);

  // Let the user pick a text file for the transcript
  QObject* file_chooser = m_app_root->findChild<QObject *>("text_file_chooser");
  QMetaObject::invokeMethod(file_chooser, "open");
}

void Transcribe::textFilePicked(const QString &url) {
  m_text_file = new QFile(QUrl(url).path());

  // Because the way the UI works, we can assume that the text is not dirty
  // So if the file exists, we load the contents into the editor window.
  if (m_text_file->exists()) {
    if (m_text_file->open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream in_stream(m_text_file);
      QVariant text(in_stream.readAll());
      QQmlProperty::write(m_text_area, "text", text);
      m_text_file->close();
      setTextDirty(false); // QML has signalled text is dirty because we changed
                           // text, so we need reset this.
    } else {
      // Todo
    }
  } else {
    // If the text file doesn't exist, empty the editor and create the file
    // by saving the text to it
    QQmlProperty::write(m_text_area, "text", QVariant(""));
    saveText();
  }

  // Update the gui
  emit textFileNameChanged();
  QQmlProperty::write(m_app_root, "is_editable", QVariant(true));
  setTextDirty(false);
}
