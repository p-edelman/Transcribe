#include "transcribe.h"

Transcribe::Transcribe(int& argc, char** argv) : QGuiApplication(argc, argv) {
  m_player = new AudioPlayer();
}

void Transcribe::guiReady(QObject* root) {
  m_app_root  = root;
  m_text_area = m_app_root->findChild<QObject *>("text_area");

  m_text_file = NULL;

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
  // Save any text that we currently might have
  saveText();
  m_text_file = NULL;

  // Open the audio file
  m_player->openAudioFile(url);

  // Let the user pick a text file for the transcript
  QObject* file_chooser = m_app_root->findChild<QObject *>("text_file_chooser");
  QMetaObject::invokeMethod(file_chooser, "open");
}

void Transcribe::textFilePicked(const QString &url) {
  m_text_file = new QFile(QUrl(url).path());

  if (m_text_file->exists()) {
    // If the text file already exists, load the text into the editor
    if (m_text_file->open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream in_stream(m_text_file);
      QVariant text(in_stream.readAll());
      QQmlProperty::write(m_text_area, "text", text);
      m_text_file->close();
    }
  } else {
    // If the text file doesn't exist, create it
    saveText();
  }

  // Update the gui
  QMetaObject::invokeMethod(m_app_root, "setFileName",
                            QGenericReturnArgument(),
                            Q_ARG(QVariant, QVariant(m_text_file->fileName())));
  QMetaObject::invokeMethod(m_app_root, "setDirty",
                            QGenericReturnArgument(),
                            Q_ARG(QVariant, QVariant(false)));
}

void Transcribe::saveText() {
  // If we don't have a file yet, open or create one
  if (!m_text_file) {
    QObject* text_file_chooser = m_app_root->findChild<QObject *>("text_file_chooser");
    QMetaObject::invokeMethod(text_file_chooser, "open");
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

  QString save_file_name;
  if (!m_text_file->isOpen()) {
    // Now give the existing file the .old extension, or .old1, .old2, etc if
    // it already exists
    if (m_text_file->exists()) {
      old_file_name = m_text_file->fileName() + ".old";
      unsigned int old_ext_counter = 1;
      while (QFile::exists(old_file_name)) {
        qDebug() << "Searching for old file name";
        save_file_name + QString::number(old_ext_counter);
      }
      if (!QFile::rename(m_text_file->fileName(), old_file_name)) {
        // TODO
        qDebug() << "Couldn't move file to .old file";
      }
    }
    // Now we can move the temp file to our actual target file
    if (QFile::rename(temp_file.fileName(), m_text_file->fileName())) {
      // Update the GUI
      QMetaObject::invokeMethod(m_app_root, "setDirty",
                                QGenericReturnArgument(),
                                Q_ARG(QVariant, QVariant(false)));

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
