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
  void init();

  void initAudioPlayer();
  void loadFile();
  void positionChangedSignal();
  void seek();
  void timeRounding();
  void stateTransitions();
};

AudioPlayerTest::AudioPlayerTest() {
  m_audio_file =  QString(SRCDIR);
  m_audio_file += "files/noise.wav";

  m_player = new AudioPlayer();
  m_player->openAudioFile(m_audio_file);
}

void AudioPlayerTest::init() {
  // Stop the audio playback
  m_player->togglePlayPause(false);

  // Reset the audio position to start of stream
  m_player->seek(SeekDirection::BACKWARD, 500);
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
  QCOMPARE(error_spy.count(), 0);
  QCOMPARE((int)player.getDuration(), 6);
}

/** Test if the positionChangedSignal is emitted during playback and if the
 *  position is reported correctly. */
void AudioPlayerTest::positionChangedSignal() {
  QSignalSpy spy(m_player, SIGNAL(positionChanged()));
  m_player->togglePlayPause(true);
  QTest::qWait(100);
  int num_signals = spy.count();
  QVERIFY(num_signals > 1); // At least one signal because of play start

  // Play for 1100 ms. This should emit exactly one signal since the 'starting
  // signals'
  QTest::qWait(1100);
  QCOMPARE(spy.count(), num_signals + 1); // Position changed signal

  m_player->togglePlayPause(false);
  QCOMPARE(spy.count(), num_signals + 2); // Signal because of play stop
  QCOMPARE((int)m_player->getPosition(), 1); // Position should be at 1 second
}

/** Test for seeking and position changed signal changed signal. */
void AudioPlayerTest::seek() {
  // We can't reuse the general AudioPlayer here, because it relies on this
  // functionality to work correctly. So we need to create a separate instance.
  AudioPlayer player;
  player.openAudioFile(m_audio_file);
  QTest::qWait(200);

  QSignalSpy spy(&player, SIGNAL(positionChanged()));

  player.togglePlayPause(true);

  // Seek forward
  int num_signals = spy.count();
  player.seek(SeekDirection::FORWARD, 2);
  QVERIFY(spy.count() > num_signals);
  QCOMPARE((int)player.getPosition(), 2);

  // Make sure we can't seek before beginning
  num_signals = spy.count();
  player.seek(SeekDirection::BACKWARD, 10);
  QVERIFY(spy.count() > num_signals);
  QCOMPARE((int)player.getPosition(), 0);

  // Make sure we can't seek past the end
  num_signals = spy.count();
  player.seek(SeekDirection::FORWARD, 20);
  QVERIFY(spy.count() > num_signals);
  QCOMPARE((int)player.getPosition(), 6);
  QTest::qWait(100);
  QCOMPARE(player.getPlayerState(), AudioPlayer::PAUSED);

  player.togglePlayPause(false);
}

void AudioPlayerTest::timeRounding() {
  // Our test file takes 5.x seconds, which should round the result to 6
  QCOMPARE((int)m_player->getDuration(), 6);

  // Make sure we're rounded to the nearest number of seconds in our position
  m_player->togglePlayPause(true);
  QTest::qWait(600);
  m_player->togglePlayPause(false);
  QCOMPARE((int)m_player->getPosition(), 1);

  m_player->togglePlayPause(true);
  QTest::qWait(600);
  m_player->togglePlayPause(false);
  QCOMPARE((int)m_player->getPosition(), 1);

  m_player->togglePlayPause(true);
  QTest::qWait(600);
  m_player->togglePlayPause(false);
  QCOMPARE((int)m_player->getPosition(), 2);
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
