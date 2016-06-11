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
  QString m_audio_file;

private Q_SLOTS:
  void initAudioPlayer();
  void loadFile();
  void stateTransitions();
};

AudioPlayerTest::AudioPlayerTest() {
  m_audio_file =  QString(SRCDIR);
  m_audio_file += "files/noise.wav";
}

/** Test the initialization of the AudioPlayer. */
void AudioPlayerTest::initAudioPlayer() {
  AudioPlayer player;
  QVERIFY(player.getPlayerState() == AudioPlayer::PAUSED);
  QVERIFY(player.getDuration() == 0);
  QVERIFY(player.getPosition() == 0);
}

/** Test loading of a simple wav file. This should result in the raising of one
 *  or more durationChanged() signals and a positive result from getDuration().
 */
void AudioPlayerTest::loadFile() {
  AudioPlayer player;
  QSignalSpy duration_spy(&player, SIGNAL(durationChanged()));
  QSignalSpy error_spy(&player, SIGNAL(audioError(const QString&)));

  player.openAudioFile(m_audio_file);
  QTest::qWait(500);

  QVERIFY(duration_spy.count() > 0);
  QCOMPARE((int)error_spy.count(), 0);
  QCOMPARE((int)player.getDuration(), 5);
}

/** Test for the accepted state transmissions. */
void AudioPlayerTest::stateTransitions() {
  AudioPlayer player;
  player.openAudioFile(m_audio_file);
  QTest::qWait(500);

  // paused -> playing: ok
  player.togglePlayPause(true);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PLAYING);

  // playing -> playing: ok
  player.togglePlayPause(true);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PLAYING);

  // playing -> waiting: ok
  player.toggleWaiting(true);
  QCOMPARE(player.getPlayerState(), AudioPlayer::WAITING);

  // waiting -> waiting: ok
  player.toggleWaiting(true);
  QCOMPARE(player.getPlayerState(), AudioPlayer::WAITING);

  // waiting -> playing: ok
  player.toggleWaiting(false);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PLAYING);

  // playing -> paused: ok
  player.togglePlayPause(false);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PAUSED);

  // paused -> paused:  ok
  player.togglePlayPause(false);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PAUSED);

  // paused -> waiting: fail
  player.toggleWaiting(true);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PAUSED);

  // waiting -> paused: ok
  player.togglePlayPause(true);
  player.toggleWaiting(true);
  QCOMPARE(player.getPlayerState(), AudioPlayer::WAITING);
  player.togglePlayPause(false);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PAUSED);
}

QTEST_MAIN(AudioPlayerTest)

#include "tst_audioplayertest.moc"
