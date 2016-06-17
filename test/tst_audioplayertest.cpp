#include "tst_audioplayertest.h"

AudioPlayerTest::AudioPlayerTest() {
  m_noise_file =  QString(SRCDIR);
  m_noise_file += "files/noise.wav";

  m_silence_file = QString(SRCDIR);
  m_silence_file += "files/silence.wav";

  m_player = new AudioPlayer();
  m_player->openFile(m_noise_file);
}

void AudioPlayerTest::init() {
  // Stop the audio playback
  m_player->togglePlayPause(false);

  // Reset the audio position to start of stream
  m_player->setPosition(0);
}

/** Test the initialization of the AudioPlayer. */
void AudioPlayerTest::initAudioPlayer() {
  AudioPlayer player;
  QVERIFY(player.getState() == AudioPlayer::PAUSED);
  QVERIFY(player.getDuration() == 0);
  QVERIFY(player.getPosition() == 0);
}

/** Test loading of a simple wav file. This should result in the raising of one
 *  or more durationChanged() signals and a positive result from getDuration().
 */
void AudioPlayerTest::loadFile() {
  AudioPlayer player;
  QSignalSpy duration_spy(&player, SIGNAL(durationChanged()));
  QSignalSpy error_spy(&player, SIGNAL(error(const QString&)));

  player.openFile(m_noise_file);
  QTest::qWait(200);

  QVERIFY(duration_spy.count() > 0);
  QCOMPARE(error_spy.count(), 0);
  QCOMPARE((int)player.getDuration(), 6);
}

/** When loading an empty file, the error() signal should be emitted. */
void AudioPlayerTest::loadErrorneousFile() {
  AudioPlayer player;
  QSignalSpy spy(&player, SIGNAL(error(const QString&)));

  QString empty_file = QString(SRCDIR);
  empty_file += "files/empty.wav";

  player.openFile(empty_file);
  QTest::qWait(200);

  QCOMPARE(spy.count(), 1);
  QList<QVariant> signal = spy.takeFirst();
  QVERIFY(signal.at(0).toString().startsWith("The audio file can't be loaded."));

  // We shouldn't be able to play the audio
  QCOMPARE(player.getState(), AudioPlayer::PAUSED);
  player.togglePlayPause(true);
  QCOMPARE(player.getState(), AudioPlayer::PAUSED);
}

/** We should be able to load a different file. After loading, we should be in
 *  the PAUSED state. */
void AudioPlayerTest::loadDifferentFile() {
  AudioPlayer player;
  QSignalSpy spy(&player, SIGNAL(durationChanged()));

  // Open the first audio file
  player.openFile(m_noise_file);
  QTest::qWait(200);
  QVERIFY(spy.count() > 0);               // durationChanged should have been emitted
  QCOMPARE((int)player.getDuration(), 6); // noise.wav has a duration of 6 seconds
  QCOMPARE(player.getState(),       // We should be initalized in paused state
           AudioPlayer::PAUSED);

  player.togglePlayPause(true);
  QCOMPARE(player.getState(), AudioPlayer::PLAYING);

  // Open alternative audio file
  int num_signals = spy.count();
  player.openFile(m_silence_file);
  QTest::qWait(200);

  QVERIFY(spy.count() > num_signals);     // durationChanged should have been emitted
  QCOMPARE((int)player.getDuration(), 7); // silence.wav has a duration of 7 seconds
  QCOMPARE(player.getState(),       // We should be initalized in paused state
           AudioPlayer::PAUSED);
}

/** Test if we can toggle between PLAYING and PAUSED state without setting the
 *  desired state explicitely (the strict sense of 'toggle') */
void AudioPlayerTest::togglePlayPause() {
  QCOMPARE(m_player->getState(), AudioPlayer::PAUSED);
  m_player->togglePlayPause();
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);
  m_player->togglePlayPause();
  QCOMPARE(m_player->getState(), AudioPlayer::PAUSED);
}

/** Test if we can toggle between PLAYING and PAUSED state by setting the
 *  desired state explicitely (the strict sense of 'toggle') */
void AudioPlayerTest::togglePlayPauseWithArgument() {
  QCOMPARE(m_player->getState(), AudioPlayer::PAUSED);
  m_player->togglePlayPause(true);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);
  m_player->togglePlayPause(false);
  QCOMPARE(m_player->getState(), AudioPlayer::PAUSED);
}

/** Test if we can toggle between PLAYING and WAITING state. */
void AudioPlayerTest::toggleWaiting() {
  m_player->togglePlayPause(true);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);

  m_player->toggleWaiting(true);
  QCOMPARE(m_player->getState(), AudioPlayer::WAITING);

  m_player->toggleWaiting(false);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);
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
  player.openFile(m_noise_file);
  QTest::qWait(200);

  QSignalSpy spy(&player, SIGNAL(positionChanged()));

  player.togglePlayPause(true);

  // Seek forward
  int num_signals = spy.count();
  player.skipSeconds(AudioPlayer::FORWARD, 4);
  QVERIFY(spy.count() > num_signals);
  QCOMPARE((int)player.getPosition(), 4);

  // Seek backward
  num_signals = spy.count();
  player.skipSeconds(AudioPlayer::BACKWARD, 2);
  QVERIFY(spy.count() > num_signals);
  QCOMPARE((int)player.getPosition(), 2);

  // Make sure we can't seek before beginning
  num_signals = spy.count();
  player.skipSeconds(AudioPlayer::BACKWARD, 10);
  QVERIFY(spy.count() > num_signals);
  QCOMPARE((int)player.getPosition(), 0);

  // Make sure we can't seek past the end
  num_signals = spy.count();
  player.skipSeconds(AudioPlayer::FORWARD, 20);
  QVERIFY(spy.count() > num_signals);
  QCOMPARE((int)player.getPosition(), 6);
  QTest::qWait(200);
  QCOMPARE(player.getState(), AudioPlayer::PAUSED);

  player.togglePlayPause(false);
}

/** Test for setting the audio position explicitely. */
void AudioPlayerTest::setPosition() {
  QCOMPARE((int)m_player->getPosition(), 0);

  m_player->setPosition(3);
  QCOMPARE((int)m_player->getPosition(), 3);
  QCOMPARE(m_player->getState(), AudioPlayer::PAUSED);

  // Make sure we can't set negative time
  m_player->setPosition(-1);
  QVERIFY(m_player->getPosition() == 0);
  QCOMPARE(m_player->getState(), AudioPlayer::PAUSED);

  // Make sure we can't set time past the end
  m_player->setPosition(100);
  QCOMPARE((int)m_player->getPosition(), 6);
  QCOMPARE(m_player->getState(), AudioPlayer::PAUSED);

  // --- Now do the same while playing
  m_player->togglePlayPause(true);

  m_player->setPosition(0);
  QCOMPARE((int)m_player->getPosition(), 0);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);

  m_player->setPosition(3);
  QCOMPARE((int)m_player->getPosition(), 3);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);

  // Make sure we can't set negative time
  m_player->setPosition(-1);
  QVERIFY(m_player->getPosition() == 0);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);

  // Make sure we can't set time past the end
  m_player->setPosition(100);
  QTest::qWait(100);
  QCOMPARE((int)m_player->getPosition(), 6);
  QCOMPARE(m_player->getState(), AudioPlayer::PAUSED);

  // Make sure we don't resume playing because just because of the seeking
  m_player->setPosition(3);
  QVERIFY(m_player->getPosition() == 3);
  QCOMPARE(m_player->getState(), AudioPlayer::PAUSED);
}

/** Test if durations and positions are properly rounded to whole seconds. */
void AudioPlayerTest::timeRounding() {
  // Our test file takes 5.8 seconds, which should round the result to 6
  QCOMPARE((int)m_player->getDuration(), 6);

  // Load the silence file, which takes 7.0 seconds and should be rounded to 7
  AudioPlayer player;
  player.openFile(m_silence_file);
  QTest::qWait(200);
  QCOMPARE((int)player.getDuration(), 7);

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

/** Test for the accepted state transmissions.
 *  This method re-tests some of the functionality previously verified. We
 *  still keep the different tests though, as they aim for different concepts
 *  which we now have out there in the open. */
void AudioPlayerTest::stateTransitions() {
  // We can't reuse the general AudioPlayer here, because it relies on this
  // functionality to work correctly. So we need to create a separate instance.
  AudioPlayer player;
  player.openFile(m_noise_file);
  QTest::qWait(200);
  QSignalSpy spy(&player, SIGNAL(stateChanged()));

  // paused -> playing: ok
  player.togglePlayPause(true);
  QCOMPARE(player.getState(), AudioPlayer::PLAYING);
  QCOMPARE(spy.count(), 1);

  // playing -> playing: ok
  player.togglePlayPause(true);
  QCOMPARE(player.getState(), AudioPlayer::PLAYING);
  QCOMPARE(spy.count(), 1); // No signal emitted

  // playing -> waiting: ok
  player.toggleWaiting(true);
  QCOMPARE(player.getState(), AudioPlayer::WAITING);
  QCOMPARE(spy.count(), 2);

  // waiting -> waiting: ok
  player.toggleWaiting(true);
  QCOMPARE(player.getState(), AudioPlayer::WAITING);
  QCOMPARE(spy.count(), 2); // No signal emitted

  // waiting -> playing: ok
  player.toggleWaiting(false);
  QCOMPARE(player.getState(), AudioPlayer::PLAYING);
  QCOMPARE(spy.count(), 3);

  // playing -> paused: ok
  player.togglePlayPause(false);
  QCOMPARE(player.getState(), AudioPlayer::PAUSED);
  QCOMPARE(spy.count(), 4);

  // paused -> paused:  ok
  player.togglePlayPause(false);
  QCOMPARE(player.getState(), AudioPlayer::PAUSED);
  QCOMPARE(spy.count(), 4); // No signal emitted

  // paused -> waiting: fail
  player.toggleWaiting(true);
  QCOMPARE(player.getState(), AudioPlayer::PAUSED);
  QCOMPARE(spy.count(), 4); // No signal emitted

  // waiting -> paused: ok
  player.togglePlayPause(true);
  player.toggleWaiting(true);
  QCOMPARE(spy.count(), 6);
  QCOMPARE(player.getState(), AudioPlayer::WAITING);
  player.togglePlayPause(false);
  QCOMPARE(player.getState(), AudioPlayer::PAUSED);
  QCOMPARE(spy.count(), 7);
}
