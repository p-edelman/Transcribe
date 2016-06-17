#ifndef TST_AUDIOPLAYERTEST_H
#define TST_AUDIOPLAYERTEST_H

#include <QDir>
#include <QtTest>
#include <QSignalSpy>

#include "audioplayer.h"

class AudioPlayerTest : public QObject {
  Q_OBJECT

public:
  AudioPlayerTest();

private:
  // The path to a wav file containing 5+ seconds of pink noise
  QString m_noise_file, m_silence_file;

  AudioPlayer* m_player;

private Q_SLOTS:
  void init();

  void initAudioPlayer();
  void loadFile();
  void loadErrorneousFile();
  void loadDifferentFile();
  void togglePlayPause();
  void togglePlayPauseWithArgument();
  void toggleWaiting();
  void positionChangedSignal();
  void seek();
  void setPosition();
  void timeRounding();
  void stateTransitions();
};

#endif // TST_AUDIOPLAYERTEST_H
