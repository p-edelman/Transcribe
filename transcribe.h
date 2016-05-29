#ifndef TRANSCRIBE_H
#define TRANSCRIBE_H

#include <QGuiApplication>

#include <audioplayer.h>
#include <keycatcher.h>

/** The main application class. */
class Transcribe : public QGuiApplication {

  Q_OBJECT

public:
  Transcribe(int &argc, char **argv);

private:
  /** The main QMediaPlayer instance for playing and seeking audio files. */
  AudioPlayer* m_player;

public slots:
  /** Callback for when the GUI is initialized and ready. It sets up all the
      bindings with the GUI. */
  void guiReady(QObject* root);
};

#endif // TRANSCRIBE_H
