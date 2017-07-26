#ifndef TRANSCRIBE_H
#define TRANSCRIBE_H

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>
#include <QSysInfo>
#include <QWindow>
#include <QQmlApplicationEngine>
#include <QQmlProperty>
#include <QSaveFile>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>
#include <QtQml>
#include <QQuickView>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniObject>
#endif

#include <functional>
#include <memory>

#include "audioplayer.h"
#include "keycatcher.h"
#include "typingtimelord.h"
#include "historymodel.h"

/** The main application class. */
class Transcribe : public QObject {

  Q_OBJECT

  /** The is_text_dirty property indicates that the current text edits are not
   *  saved. */
  Q_PROPERTY(bool is_text_dirty
             READ isTextDirty
             WRITE setTextDirty
             NOTIFY textDirtyChanged)

  /** The text_file_name property is a read-only property that holds the
   *  actual file name of the text file, but not the path. It can be used
   *  for GUI purposes. */
  Q_PROPERTY(QString text_file_name
             READ getTextFileName
             NOTIFY textFileNameChanged)

  /** The num_words property is a read-only property that holds the number of
   *  words in the text area. */
  Q_PROPERTY(uint num_words
             READ getNumWords
             NOTIFY numWordsChanged)

public:
  Transcribe(QObject* parent = 0);
  ~Transcribe();

  /** Open the audio file specified by the path. This will unload the text file
   *  from the editor. */
  void openAudioFile(const QString& path);

  /** Open the text file specified by the path. If it is an existing file, the
   *  text will be loaded into the editor, otherwise a new file will be created.
   */
  void openTextFile(const QString& path);

  /** Indicate that the text is dirty, thus that the current edits are not
   *  saved. If it changes, the textDirtyChanged() signal will be emitted. */
  void setTextDirty(bool is_dirty);
  bool isTextDirty();

  /** Return the file name (but not the path) of the transcript text file. */
  QString getTextFileName() const;

  /** Return the number of words in the text editor. */
  uint getNumWords();

public slots:
  /** Save the text in the GUI to m_text_file.
      @return true if the file is saved, false otherwise. */
  bool saveText();

  /** Communicate an error to the end user by displaying an error dialog with
   *  the message. All other modal dialogs will be closed.
   *  @param message the message to display to the end user. */
  void errorDetected(const QString& message);

  /** Shut down the application. If the text is dirty, ask the user if it should
   *  be saved. */
  void close();

signals:
  void textDirtyChanged(bool is_dirty);
  void textFileNameChanged();
  void numWordsChanged();

private:
  /** Whether the text is dirty, thus the current edits in the transcription
   *  text are not changed. */
  bool m_is_text_dirty = false;

  /** The number of words in the text area. */
  uint m_num_words = 0;

  /** The main AudioPlayer instance for playing and seeking audio files. */
  std::shared_ptr<AudioPlayer> m_player;

  /** A TypingTimeLord to let the typing of the user control the audio playback.
   */
  TypingTimeLord m_keeper;

  /** The QML connection. */
  QQmlApplicationEngine m_engine;

  /** The main ApplicationWindow */
  QWindow* m_main_window;

  /** The text_area TextEdit that holds the transcript. */
  QObject* m_text_area;

  /** The text file where the transcript needs to be written to. */
  QFile* m_text_file = NULL;

  /** The history items. */
  HistoryModel m_history;

  /** If we're opening an audio file from history, it has an associated
   *  audio position. We have to wait until the AudioPlayer has loaded the file
   *  and has emitted the durationChanged() signal before we can set this
   *  position, so we'll save it until this has happened. */
  qint64 m_restore_pos = 0;

  /** The keys for the entries in the configuration file. */
  const QString CFG_GROUP_SCREEN        = "screen";
  const QString CFG_SCREEN_SIZE         = "size";
  const QString CFG_SCREEN_POS          = "pos";
  const QString CFG_SCREEN_IS_MAXIMIZED = "is_maximized";

private slots:
  /** Callback for when the GUI is initialized and ready. It sets up all the
      bindings with the GUI. */
  void guiReady(QObject* root);

  /** Callback for when the AudioPlayer signals that the duration of the media
   *  has changed. This is basically the callback for when an audio file has
   *  loaded, since that is when the media duration changes. */
  void mediaDurationChanged();

  /** Signal that the user wants to start with a new project. It will first open
   *  the audio file dialog, and then open the text file dialog. */
  void pickFiles();

#ifdef Q_OS_ANDROID
  void shareText();
#endif

  /** Callback for when the user has picked an item from the history menu. It
   *  will query m_history for the proper file names and load them.
   *  @param index the index in the HistoryModel of the files to be loaded. */
  void restoreHistory(int index);

  /** Save the newly loaded files to the history model.
   *  Because audio loading is asynchronous, this method should both be bound to
   *  a signal for audio loading AND be called after text loading. The history
   *  item will only be saved if a text file is loaded. Whether audio needs to
   *  be available is determined by the allow_text_only parameter.
   *  @param allow_text_only if false, the history item will only be saved if
   *                         the audio is availaible. */
  void saveHistory(bool allow_text_only = false);

  /** Count the number of words in the text area.
   *  This is a naïve count; everything between space characters is regarded a
   *  word.
   *  When the number of words is changed, the numWordsChanged() signal is
   *  emitted. */
  void countWords();
};

#endif // TRANSCRIBE_H
