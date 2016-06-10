#ifndef TRANSCRIBE_H
#define TRANSCRIBE_H

#include <QFileDialog>
#include <QFileInfo>
#include <QApplication>
#include <QQmlProperty>
#include <QTemporaryFile>

#include <audioplayer.h>
#include <keycatcher.h>

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

  /** Indicate that the text is dirty, thus that the current edits are not
   *  saved. If it changes, the textDirtyChanged() signal will be emitted. */
  void setTextDirty(bool is_dirty);
  bool isTextDirty();

  /** Return the file name (but not the path) of the transcript text file. */
  QString getTextFileName() const;

public slots:
  /** Save the text in the GUI to m_text_file. */
  void saveText();

signals:
  void textDirtyChanged(bool is_dirty);
  void textFileNameChanged();

private:
  /** Whether the text is dirty, thus the current edits in the transcription
   *  text are not changed. */
  bool m_is_text_dirty = false;

  /** The main QMediaPlayer instance for playing and seeking audio files. */
  AudioPlayer* m_player;

  /** The main ApplicationWindow */
  QObject* m_app_root;

  /** The text_area TextEdit that holds the transcript. */
  QObject* m_text_area;

  /** The text file where the transcript needs to be written to. */
  QFile* m_text_file;

  /** Open the text file specified by the path. If it is an existing file, the
   *  text will be loaded into the editor, otherwise a new file will be created.
   */
  void openTextFile(const QString& path);

private slots:
  /** Callback for when the GUI is initialized and ready. It sets up all the
      bindings with the GUI. */
  void guiReady(QObject* root);

  /** Signal that the user wants to start with a new project. It will first open
   *  the audio file dialog, and then open the text file dialog. */
  void pickFiles();
};

#endif // TRANSCRIBE_H
