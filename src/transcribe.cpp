#include "transcribe.h"

Transcribe::Transcribe(int &argc, char **argv) :
  QApplication(argc, argv),
  m_player(new AudioPlayer(this), std::mem_fn(&AudioPlayer::deleteLater)),
  m_keeper(m_player),
  m_engine(this) {

  connect(m_player.get(), SIGNAL(error(const QString&)),
          this,           SLOT(errorDetected(const QString&)));

  // Expose the Transcribe object to the gui for setting and getting properties
  // and such
  m_engine.rootContext()->setContextProperty("app",    this);
  m_engine.rootContext()->setContextProperty("player", m_player.get());

  // This is a bit of quirkiness of Qt; you can't declare an enum as a QML type,
  // but you can declare a C++ class with a public enum as a QML library, and
  // then access the enum values as properties. So we expose the "AudioPlayer"
  // (the class) as "PlayerState" in QML.
  qmlRegisterType<AudioPlayer>("AudioPlayer", 1, 0, "PlayerState");

  // Load the GUI. When it is ready, the guiReady() method takes over.
  QObject::connect(&m_engine, SIGNAL(objectCreated(QObject*, QUrl)),
                   this,      SLOT(guiReady(QObject*)));
  m_engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
}

Transcribe::~Transcribe() {
  if (m_text_file) m_text_file->deleteLater();
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

void Transcribe::openAudioFile(const QString& path) {
  // Unload the current text file
  m_text_file = NULL;
  emit textFileNameChanged();
  QQmlProperty::write(m_app_root, "is_editable", QVariant(false));

  // Open the audio file
  m_player->openFile(path);
}

bool Transcribe::saveText() {
  if (!m_text_file) {
    // This shouldn't happen because of the GUI. We silently ignore it
    qDebug() << "There is no text file loaded";
    return false;
  }

  QSaveFile file(m_text_file->fileName());
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out_stream(&file);
    out_stream << QQmlProperty::read(m_text_area, "text").toString();
    if (file.commit()) {
      setTextDirty(false);
      return true;
    }
  }

  // Notify that the file could not be saved
  QString general_msg =  tr("There was an error saving the text file.\n");
  general_msg         += tr("The latest changes are not saved!");

  errorDetected(general_msg);
  return false;
}

void Transcribe::errorDetected(const QString& message) {
  // Pause the audio
  m_player->togglePlayPause(false);

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

  // Set the icon, which, strangely enough, cannot be done from QML
  ((QWindow*)root)->setIcon(QIcon("://window_icon"));

  // Install the key filter and connect its signals
  KeyCatcher* catcher = new KeyCatcher(root);
  connect(catcher,   SIGNAL(keyTyped()),
          &m_keeper, SLOT(keyTyped()));
  connect(catcher, SIGNAL(saveFile()),
          this,    SLOT(saveText()));
  connect(catcher,        SIGNAL(seekAudio(AudioPlayer::SeekDirection,int)),
          m_player.get(), SLOT(skipSeconds(AudioPlayer::SeekDirection, int)));
  connect(catcher,        SIGNAL(togglePlayPause()),
          m_player.get(), SLOT(togglePlayPause()));
  connect(catcher,        SIGNAL(togglePlayPause(bool)),
          m_player.get(), SLOT(togglePlayPause(bool)));
  connect(catcher,        SIGNAL(boost(bool)),
          m_player.get(), SLOT(boost(bool)));
  root->installEventFilter(catcher);

  // Attach audio controls to the AudioPlayer
  QObject* controls = root->findChild<QObject *>("media_controls");
  connect(controls,       SIGNAL(valueChanged(int)),
          m_player.get(), SLOT(setPosition(int)));
  connect(controls,       SIGNAL(playingStateChanged(bool)),
          m_player.get(), SLOT(togglePlayPause(bool)));

  // Connect GUI events to their callbacks
  connect(m_app_root, SIGNAL(saveText()),
          this, SLOT(saveText()));
  connect(m_app_root, SIGNAL(pickFiles()),
          this, SLOT(pickFiles()));
  connect(m_app_root, SIGNAL(signalQuit()),
          this, SLOT(close()));
}

void Transcribe::pickFiles() {
  QFileDialog dlg;

  // Unfortunately, QFileDialog on Android looks horrible, but we can make it
  // a bit better by maximizing it.
  if (QSysInfo::productType() == "android") {
    dlg.setWindowState(Qt::WindowMaximized);
    dlg.setViewMode(QFileDialog::List);
  }

  // Let the user pick an audio file
  dlg.setWindowTitle(tr("Open an audio file"));
  dlg.setNameFilter(tr("Audio files (*.wav *.mp3 *.aac *.amr *.aiff *.flac *.ogg *.wma)"));
  dlg.setFileMode(QFileDialog::ExistingFile);
  dlg.setAcceptMode(QFileDialog::AcceptOpen);
  if (dlg.exec() == QDialog::Rejected || dlg.selectedFiles().count() != 1) {
    return;
  }

  openAudioFile(dlg.selectedFiles().at(0));

  // Recycle the file dialog to let the user pick a text file for the
  // transcript. As a file suggestion, we base a txt file on the current audio
  // file.
  QFileInfo info(dlg.selectedFiles().at(0));
  QString suggestion = info.absolutePath() + "/" + info.baseName() + ".txt";
  dlg.setWindowTitle(tr("Pick a text file for the transcript"));
  dlg.setNameFilter(tr("Text files (*.txt)"));
  dlg.setFileMode(QFileDialog::AnyFile);
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setOption(QFileDialog::DontConfirmOverwrite, true);
  dlg.setLabelText(QFileDialog::Accept, tr("Open/Create"));
  dlg.selectFile(suggestion);
  if (dlg.exec() == QDialog::Rejected || dlg.selectedFiles().count() != 1) {
    return;
  }

  openTextFile(dlg.selectedFiles().at(0));
}

void Transcribe::openTextFile(const QString& path) {
  // Because the way the UI works, we can assume that the text is not dirty
  // So if the file exists, we load the contents into the editor window.
  if (m_text_file) {
    m_text_file->deleteLater();
  }
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
