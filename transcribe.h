#ifndef TRANSCRIBE_H
#define TRANSCRIBE_H

#include <QGuiApplication>

#include <audioplayer.h>

/** The main application class. */
class Transcribe : public QGuiApplication {

  Q_OBJECT

public:
  Transcribe(int &argc, char **argv);

private:
  /** The main QMediaPlayer instance for playing and seeking audio files. */
  AudioPlayer* m_player;

public slots:
  void guiReady(QObject* root);
};

#endif // TRANSCRIBE_H
