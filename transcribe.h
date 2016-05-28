#ifndef TRANSCRIBE_H
#define TRANSCRIBE_H

#include <QGuiApplication>
#include <QMediaPlayer>

/** The main application class. */
class Transcribe : public QGuiApplication {

public:
  Transcribe(int &argc, char **argv);

private:
  /** The main QMediaPlayer instance for playing and seeking audio files. */
  QMediaPlayer* m_player;
};

#endif // TRANSCRIBE_H
