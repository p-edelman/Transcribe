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
#include <QScreen>
#include <QTemporaryFile>
#include <QtQml>
#include <QQuickView>

#include "audioplayer.h"
#include "keycatcher.h"
#include "typingtimelord.h"

/** The main application class. */
class Transcribe : public QApplication {

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

public:
  Transcribe(int &argc, char **argv);

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

private:
  /** Whether the text is dirty, thus the current edits in the transcription
   *  text are not changed. */
  bool m_is_text_dirty = false;

  /** The main AudioPlayer instance for playing and seeking audio files. */
  AudioPlayer* m_player;

  /** A TypingTimeLord to let the typing of the user control the audio playback.
   */
  TypingTimeLord* m_keeper;

  /** The main ApplicationWindow */
  QObject* m_app_root;

  /** The text_area TextEdit that holds the transcript. */
  QObject* m_text_area;

  /** The text file where the transcript needs to be written to. */
  QFile* m_text_file;

private slots:
  /** Callback for when the GUI is initialized and ready. It sets up all the
      bindings with the GUI. */
  void guiReady(QObject* root);

  /** Signal that the user wants to start with a new project. It will first open
   *  the audio file dialog, and then open the text file dialog. */
  void pickFiles();
};

#endif // TRANSCRIBE_H
