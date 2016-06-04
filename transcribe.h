#ifndef TRANSCRIBE_H
#define TRANSCRIBE_H

#include <QGuiApplication>
#include <QQmlProperty>
#include <QTemporaryFile>

#include <audioplayer.h>
#include <keycatcher.h>

/** The main application class. */
class Transcribe : public QGuiApplication {

  Q_OBJECT

public:
  Transcribe(int &argc, char **argv);

public slots:
  /** Save the text to m_text_file. If m_text_file is NULL, a file picker will
   *  be presented to the user first. */
  void saveText();

private:
  /** The main QMediaPlayer instance for playing and seeking audio files. */
  AudioPlayer* m_player;

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

  void audioFilePicked(const QString &url);
  void textFilePicked(const QString &url);
};

#endif // TRANSCRIBE_H
