#include "transcribe.h"

Transcribe::Transcribe(QObject* parent) :
  QObject(parent),
  m_player(new AudioPlayer(this), std::mem_fn(&AudioPlayer::deleteLater)),
  m_keeper(m_player),
  m_engine(this) {

  connect(m_player.get(), SIGNAL(error(const QString&)),
          this,           SLOT(errorDetected(const QString&)));
  connect(m_player.get(), SIGNAL(durationChanged()),
          this,           SLOT(mediaDurationChanged()));

  // Expose the various objects to the gui for setting and getting properties
  // and such
  m_engine.rootContext()->setContextProperty("app",            this);
  m_engine.rootContext()->setContextProperty("player",         m_player.get());
  m_engine.rootContext()->setContextProperty("typingtimelord", &m_keeper);
  m_engine.rootContext()->setContextProperty("history",        &m_history);

  // This is a bit of quirkiness of Qt; you can't declare an enum as a QML type,
  // but you can declare a C++ class with a public enum as a QML library, and
  // then access the enum values as properties. So we expose the "AudioPlayer"
  // (the class) as "PlayerState" in QML.
  qmlRegisterType<AudioPlayer>("AudioPlayer", 1, 0, "PlayerState");

  // Load the GUI. When it is ready, the guiReady() method takes over.
  connect(&m_engine, SIGNAL(objectCreated(QObject*, QUrl)),
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

uint Transcribe::getNumWords() {
  return m_num_words;
}

void Transcribe::openAudioFile(const QString& path) {
  // Unload the current text file
  m_text_file = NULL;
  emit textFileNameChanged();
  QQmlProperty::write(m_main_window, "is_editable", QVariant(false));

  // Open the audio file
  m_player->openFile(path);
}

bool Transcribe::saveText() {
  if (!m_text_file) {
    // This shouldn't happen because of the GUI. We silently ignore it
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

  // If we were loaded with a position to restore, invalidate it
  m_restore_pos = 0;

  // Close any open modal windows. This is especially useful for an error with
  // audio loading, in which case the text file dialog is still active.
  QWidget* modal = QApplication::activeModalWidget();
  while (modal != NULL) {
    modal->close();
    modal = QApplication::activeModalWidget();
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
#ifndef Q_OS_ANDROID
    // Save window state
    QSettings settings;
    settings.beginGroup(CFG_GROUP_SCREEN);
    if (m_main_window->visibility() == QWindow::Maximized) {
      settings.setValue(CFG_SCREEN_IS_MAXIMIZED, true);
    } else {
      settings.setValue(CFG_SCREEN_IS_MAXIMIZED, false);
    }
    settings.setValue(CFG_SCREEN_SIZE, m_main_window->size());
    settings.setValue(CFG_SCREEN_POS,  m_main_window->position());
    settings.endGroup();\
    settings.sync();
#endif

    saveHistory();

    QApplication::quit();
  }
}

#ifdef Q_OS_ANDROID
void Transcribe::connectVirtualKeyboard() {
  if (qApp->inputMethod()->isVisible()) {
    // The virtual keyboard is visible. The only way to detect typing is by
    // observing changes in the text area, the KeyCatcher doesn't respond to
    // it.
    connect(m_text_area, SIGNAL(textChanged()),
            &m_keeper,   SLOT(keyTyped()));
  } else {
    // The virtual keyboard is not visible, so there's a physical keyboard that
    // is intercepted by the KeyCatcher and listening to text changes as well
    // would produce double signals.
    // Listening to the physical keyboard has a bit broader scope than the
    // virtual keyboard since it also responds to keyboard navigation and
    // such.
    disconnect(m_text_area, SIGNAL(textChanged()),
               &m_keeper,   SLOT(keyTyped()));
  }
}
#endif

void Transcribe::guiReady(QObject* root) {
  m_main_window = qobject_cast<QWindow*>(root);
  m_text_area = m_main_window->findChild<QObject*>("text_area");

  // Set the icon, which, strangely enough, cannot be done from QML
  m_main_window->setIcon(QIcon("://window_icon"));

#ifndef Q_OS_ANDROID
  // Restore window state
  QSettings settings;
  settings.beginGroup(CFG_GROUP_SCREEN);
  if (settings.value(CFG_SCREEN_IS_MAXIMIZED, false).toBool()) {
    m_main_window->setVisibility(QWindow::Maximized);
  } else {
    m_main_window->resize(settings.value(CFG_SCREEN_SIZE,
                                         QSize(640, 480)).toSize());
    m_main_window->setPosition(settings.value(CFG_SCREEN_POS,
                                              QPoint(200, 200)).toPoint());
    settings.endGroup();\
  }
#endif

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
#ifdef Q_OS_ANDROID
  // On Android, we might connect the signals when using the virtual keyboard
  // in addition to the signals from the physical keyboard.
  connect(qApp->inputMethod(), SIGNAL(visibleChanged()),
          this,                SLOT(connectVirtualKeyboard()));
  connectVirtualKeyboard();
#endif

  // Connect GUI events to their callbacks
  connect(m_main_window,  SIGNAL(audioPositionChanged(int)),
          m_player.get(), SLOT(setPosition(int)));
  connect(m_main_window,  SIGNAL(playingStateChanged(bool)),
          m_player.get(), SLOT(togglePlayPause(bool)));
  connect(m_main_window, SIGNAL(saveText()),
          this,          SLOT(saveText()));
#ifdef Q_OS_ANDROID
  connect(m_main_window, SIGNAL(shareText()),
          this,          SLOT(shareText()));
  connect(m_main_window, SIGNAL(deleteText()),
          this,          SLOT(deleteText()));
#endif
  connect(m_main_window, SIGNAL(pickFiles()),
          this,          SLOT(pickFiles()));
  connect(m_main_window, SIGNAL(historySelected(int)),
          this,          SLOT(restoreHistory(int)));
  connect(m_main_window, SIGNAL(signalQuit()),
          this,          SLOT(close()));
  connect(m_main_window, SIGNAL(numWordsDirty()),
          this,          SLOT(countWords()));
}

void Transcribe::mediaDurationChanged() {
  if (m_player->getDuration() > 0) {

    if (m_restore_pos > 0) {
      // We were loaded with a position to restore
      m_player->setPosition(m_restore_pos);
      m_restore_pos = 0;
    }

    // Now that the audio file is fully loaded, we can save the state (and send
    // the current configuration to the top).
    saveHistory();
  }
}

void Transcribe::pickFiles() {
  QFileDialog dlg;

  // Unfortunately, Android doesn't really work with the concept of files,
  // they are abstracted away. Since it would require a major effort to make
  // this work in the Android way, we'll just try to make the best of it.
#ifdef Q_OS_ANDROID
    // Make the QFileDialog a bit better by maximizing it.
    dlg.setWindowState(Qt::WindowMaximized);
    dlg.setViewMode(QFileDialog::List);

    // Add the root and the internal memory location to the paths to choose
    // from. There are no real standard paths for this, let's hope Qt knows
    // what to do.
    QUrl home_url = QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first());
    dlg.setDirectoryUrl(home_url);
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/");
    urls << home_url;
    dlg.setSidebarUrls(urls);
#endif

  // Let the user pick an audio file
  dlg.setWindowTitle(tr("Open an audio file"));
  dlg.setNameFilter(tr("Audio files (*.wav *.mp3 *.aac *.amr *.aiff *.flac *.ogg *.wma)"));
  dlg.setFileMode(QFileDialog::ExistingFile);
  dlg.setAcceptMode(QFileDialog::AcceptOpen);
  if (dlg.exec() == QDialog::Rejected || dlg.selectedFiles().count() != 1) {
    return;
  }

  m_restore_pos = 0;
  openAudioFile(dlg.selectedFiles().at(0));

#ifdef Q_OS_ANDROID
  QString audio_path = dlg.selectedFiles().at(0);
  QString text_path;

  // Check if the audio file is in our history
  if (!m_history.textFileForAudio(audio_path, text_path)) {
    // If not, create a new file in the app private folder based on the audio
    // file name. If a text file with the name already exists, append a number
    // to it.
    QString base_name = QFileInfo(audio_path).baseName();
    QDir home = QDir(QStandardPaths::writableLocation((QStandardPaths::AppDataLocation)));
    text_path = home.filePath(base_name + ".txt");
    short counter = 1;
    while (QFile::exists(text_path)) {
      text_path = home.filePath(QString("%1_%2.txt").arg(base_name).arg(counter, 2, 10, QChar('0')));
      counter++;
    }
  }

  openTextFile(text_path);
#else
  // Recycle the file dialog to let the user pick a text file for the
  // transcript. As a file suggestion, we base a txt file on the current audio
  // file.
  dlg.setWindowTitle(tr("Pick a text file for the transcript"));
  dlg.setNameFilter(tr("Text files (*.txt)"));
  dlg.setFileMode(QFileDialog::AnyFile);
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setOption(QFileDialog::DontConfirmOverwrite, true);
  dlg.setLabelText(QFileDialog::Accept, tr("Open/Create"));
  QFileInfo info(dlg.selectedFiles().at(0));
  dlg.setDirectory(info.absolutePath());
  dlg.selectFile(info.baseName() + ".txt");
  if (dlg.exec() == QDialog::Rejected || dlg.selectedFiles().count() != 1) {
    return;
  }

  openTextFile(dlg.selectedFiles().at(0));
#endif

  // saveHistory() is called when the audio file has finished loading, but we
  // need do it here as well because openTextFile() might return after the audio
  // file has finished loading. The joys of concurrency ...
  saveHistory();
}

#ifdef Q_OS_ANDROID
  void Transcribe::shareText() {
    QAndroidJniObject::callStaticMethod<void>(
          "org/mrpi/Transcribe/TranscriptionSharer",
          "shareTranscription",
          "(Landroid/app/Activity;Ljava/lang/String;)V",
          QtAndroid::androidActivity().object(),
          QAndroidJniObject::fromString(QQmlProperty::read(m_text_area, "text").toString()).object());
  }

  void Transcribe::deleteText() {
    if (m_text_file) {
      // Let's ask for confirmation first
      QMessageBox box;
      box.setText(tr("Do you really want to delete the transcription text?"));
      box.setInformativeText(tr("There's no way to get it back.\nThe audio file will not be deleted."));
      box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

      if (box.exec() == QMessageBox::Yes) {
        // Delete the file from our history model
        m_history.del(HistoryModel::TextFileRole, m_text_file);

        // Clear and lock the text area
        QQmlProperty::write(m_text_area,
                            "text",
                            QVariant::fromValue(QString()));
        QQmlProperty::write(m_main_window,
                            "is_editable",
                            QVariant(false));
        setTextDirty(false);

        // Actually remove the file from disk
        m_text_file->remove();

        // Update the text file parameter
        m_text_file->deleteLater();
        m_text_file = NULL;
        emit textFileNameChanged();
      }
    }
  }
#endif

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
  QQmlProperty::write(m_main_window, "is_editable", QVariant(true));
  setTextDirty(false);
}

void Transcribe::restoreHistory(int index) {
  saveHistory();

  QModelIndex model_index = m_history.index(index, 0);
  m_restore_pos = model_index.data(HistoryModel::AudioPostionRole).toUInt();
  QString audio_file_path = model_index.data(HistoryModel::AudioFileRole).toString();
  QString text_file_path  = model_index.data(HistoryModel::TextFileRole).toString();
  openAudioFile(audio_file_path);
  openTextFile(text_file_path);
}

void Transcribe::saveHistory(bool allow_text_only) {
  if ((m_text_file) &&
      (m_player->getDuration() > 0 || allow_text_only)) {

    m_history.add(QFileInfo(*m_text_file).absoluteFilePath(),
                  m_player->getFilePath(),
                  m_player->getPosition());
  }
}

void Transcribe::countWords() {
  // Get the text from the editor
  QString text = QQmlProperty::read(m_text_area, "text").toString();

  // We simply iterate over all characters, and count every transition from a
  // 'space' (space, newline, tab) to a 'non-space' a new word.
  bool in_word   = false;
  uint num_words = 0;
  for (int i = 0; i < text.length(); i++) {
    if (text[i].isSpace()) {
      if (in_word) {
        in_word = false;
      }
    } else {
      if (!in_word) {
        in_word = true;
        num_words++;
      }
    }
  }

  // If the number of words has changed, emit a signal.
  if (num_words != m_num_words) {
    m_num_words = num_words;
    emit numWordsChanged();
  }
}
