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

  AudioPlayer* m_player;

private Q_SLOTS:
  void initTestCase();

  void initAudioPlayer();
  void loadFile();
  void seekAndSignal();
  void stateTransitions();
};

AudioPlayerTest::AudioPlayerTest() {
  m_audio_file =  QString(SRCDIR);
  m_audio_file += "files/noise.wav";

  m_player = new AudioPlayer();
  m_player->openAudioFile(m_audio_file);
}

void AudioPlayerTest::initTestCase() {
  // Stop the audio playback
  m_player->togglePlayPause(false);

  // Reset the audio position to start of stream
  m_player->seek(SeekDirection::BACKWARD, 10);
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
  QTest::qWait(200);

  QVERIFY(duration_spy.count() > 0);
  QCOMPARE((int)error_spy.count(), 0);
  QCOMPARE((int)player.getDuration(), 5);
}

/** Test for seeking and position changed signal changed signal. */
void AudioPlayerTest::seekAndSignal() {
  // We can't reuse the general AudioPlayer here, because it relies on this
  // functionality to work correctly. So we need to create a separate instance.
  AudioPlayer player;
  player.openAudioFile(m_audio_file);
  QTest::qWait(200);


  QSignalSpy spy(&player, SIGNAL(positionChanged()));
  player.togglePlayPause(true);
  // Let it play for 1100 ms. This should emit at least one signal (the exact
  // number is undetermined, as it might be emitted several times when
  // starting).
  QTest::qWait(1100);
  player.togglePlayPause(false);
  QVERIFY(spy.count() > 0);
  QCOMPARE((int)player.getPosition(), 1);

  player.togglePlayPause(true);

  // Make sure we can't seek before beginning
  int num_signals = spy.count();
  player.seek(SeekDirection::BACKWARD, 10);
  QVERIFY(spy.count() > num_signals);
  QCOMPARE((int)player.getPosition(), 0);

  // Seek forward
  num_signals = spy.count();
  player.seek(SeekDirection::FORWARD, 2);
  QVERIFY(spy.count() > num_signals);
  QCOMPARE((int)player.getPosition(), 2);

  // Make sure we can't seek past the end
  num_signals = spy.count();
  player.seek(SeekDirection::FORWARD, 20);
  QVERIFY(spy.count() > num_signals);
  QCOMPARE((int)player.getPosition(), 5);
  QTest::qWait(100);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PAUSED);

  player.togglePlayPause(false);
}

/** Test for the accepted state transmissions. */
void AudioPlayerTest::stateTransitions() {
  // We can't reuse the general AudioPlayer here, because it relies on this
  // functionality to work correctly. So we need to create a separate instance.
  AudioPlayer player;
  player.openAudioFile(m_audio_file);
  QTest::qWait(200);
  QSignalSpy spy(&player, SIGNAL(playerStateChanged()));

  // paused -> playing: ok
  player.togglePlayPause(true);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PLAYING);
  QCOMPARE(spy.count(), 1);

  // playing -> playing: ok
  player.togglePlayPause(true);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PLAYING);
  QCOMPARE(spy.count(), 1); // No signal emitted

  // playing -> waiting: ok
  player.toggleWaiting(true);
  QCOMPARE(player.getPlayerState(), AudioPlayer::WAITING);
  QCOMPARE(spy.count(), 2);

  // waiting -> waiting: ok
  player.toggleWaiting(true);
  QCOMPARE(player.getPlayerState(), AudioPlayer::WAITING);
  QCOMPARE(spy.count(), 2); // No signal emitted

  // waiting -> playing: ok
  player.toggleWaiting(false);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PLAYING);
  QCOMPARE(spy.count(), 3);

  // playing -> paused: ok
  player.togglePlayPause(false);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PAUSED);
  QCOMPARE(spy.count(), 4);

  // paused -> paused:  ok
  player.togglePlayPause(false);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PAUSED);
  QCOMPARE(spy.count(), 4); // No signal emitted

  // paused -> waiting: fail
  player.toggleWaiting(true);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PAUSED);
  QCOMPARE(spy.count(), 4); // No signal emitted

  // waiting -> paused: ok
  player.togglePlayPause(true);
  player.toggleWaiting(true);
  QCOMPARE(spy.count(), 6);
  QCOMPARE(player.getPlayerState(), AudioPlayer::WAITING);
  player.togglePlayPause(false);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PAUSED);
  QCOMPARE(spy.count(), 7);
}

QTEST_MAIN(AudioPlayerTest)

#include "tst_audioplayertest.moc"
