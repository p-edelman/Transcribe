#include "typingtimelordtest.h"

TypingTimeLordTest::TypingTimeLordTest() {
  QString silence_file =  SRCDIR;
  silence_file         += "files/silence.wav";
  m_player = std::shared_ptr<AudioPlayer>(new AudioPlayer());
  m_player->openFile(silence_file);
  QTest::qWait(200);

  m_time_lord = new TypingTimeLord(m_player);
}

void TypingTimeLordTest::init() {
  // We set the timeouts to the minimal values for testing
  m_time_lord->setWaitTimeout(2000);
  m_time_lord->setTypeTimeout(500);

  // Reset the AudioPlayer
  m_player->togglePlayPause(false);
  m_player->setPosition(0);
}

/** If a key is never typed, the audio should always continue playing */
void TypingTimeLordTest::keepPlayingWithNoTyping() {
  m_player->togglePlayPause(true);
  QTest::qWait(1000);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);
}

/** If the user is tpying below the m_type_timeout treshold, the audio should
 *  keep playing. */
void TypingTimeLordTest::keepPlayingWithSlowTyping() {
  m_player->togglePlayPause(true);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);

  m_time_lord->keyTyped();
  for (int i = 0; i < 3; i++) {
    QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);
    QTest::qWait(110);
    m_time_lord->keyTyped();
  }
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);
}

/** Test that the audio is stopped if the user keeps typing after the wait
 *  timeout, and that playing resumes after type timeout. */
void TypingTimeLordTest::waitAndUnwait() {
  m_player->togglePlayPause(true);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);

  // Start typing within the type timer interval
  m_time_lord->keyTyped();
  for (int i = 0; i < 5; i++) {
    QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);
    QTest::qWait(450);
    m_time_lord->keyTyped();
  }

  // Wait timer should have run out
  QCOMPARE(m_player->getState(), AudioPlayer::WAITING);

  // Keep typing within the type timer interval
  for (int i = 0; i < 4; i++) {
    QTest::qWait(400);
    m_time_lord->keyTyped();
  }
  QCOMPARE(m_player->getState(), AudioPlayer::WAITING);

  // Stop typing
  QTest::qWait(600);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);
}

/** If the user starts typing and stops typing before the pause timer runs out,
 *  the pause timer should not start again until the user starts typing again.
 */
void TypingTimeLordTest::resetWaitTimerOnTypingPause() {
  m_player->togglePlayPause(true);

  // Start typing but stop before the wait timer runs out
  m_time_lord->keyTyped();
  for (int i = 0; i < 2; i++) {
    QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);
    QTest::qWait(400);
    m_time_lord->keyTyped();
  }
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);

  // Now wait a bit
  QTest::qWait(600);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);

  // Start typing again until the wait timer runs out
  m_time_lord->keyTyped();
  for (int i = 0; i < 5; i++) {
    QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);
    QTest::qWait(450);
    m_time_lord->keyTyped();
  }
  QCOMPARE(m_player->getState(), AudioPlayer::WAITING);
}

void TypingTimeLordTest::setTimeouts() {
  m_time_lord->setTypeTimeout(TypingTimeLord::TYPE_TIMEOUT_MIN - 100);
  QCOMPARE(m_time_lord->getTypeTimeout(), (int)TypingTimeLord::TYPE_TIMEOUT_MIN);
  m_time_lord->setTypeTimeout(TypingTimeLord::TYPE_TIMEOUT_MAX + 100);
  QCOMPARE(m_time_lord->getTypeTimeout(), (int)TypingTimeLord::TYPE_TIMEOUT_MAX);
  m_time_lord->setTypeTimeout(TypingTimeLord::TYPE_TIMEOUT_MIN + 100);
  QCOMPARE(m_time_lord->getTypeTimeout(), (int)TypingTimeLord::TYPE_TIMEOUT_MIN + 100);

  m_time_lord->setWaitTimeout(TypingTimeLord::WAIT_TIMEOUT_MIN - 100);
  QCOMPARE(m_time_lord->getWaitTimeout(), (int)TypingTimeLord::WAIT_TIMEOUT_MIN);
  m_time_lord->setWaitTimeout(TypingTimeLord::WAIT_TIMEOUT_MAX + 100);
  QCOMPARE(m_time_lord->getWaitTimeout(), (int)TypingTimeLord::WAIT_TIMEOUT_MAX);
  m_time_lord->setWaitTimeout(TypingTimeLord::WAIT_TIMEOUT_MIN + 100);
  QCOMPARE(m_time_lord->getWaitTimeout(), (int)TypingTimeLord::WAIT_TIMEOUT_MIN + 100);
}

void TypingTimeLordTest::dontSetTooSmallPauseTimeout() {
  m_time_lord->setWaitTimeout(3000);
  m_time_lord->setTypeTimeout(4000);
  QCOMPARE(m_time_lord->getWaitTimeout(), 3000);
  QCOMPARE(m_time_lord->getTypeTimeout(), 2900); // Can't set larger than wait
                                                 // timeout

  m_time_lord->setWaitTimeout(2000);
  QCOMPARE(m_time_lord->getWaitTimeout(), 3000); // Can't set smaller than type
                                                 // timeout
}

void TypingTimeLordTest::changeTimeouts() {
  m_time_lord->setWaitTimeout(3000); // 1 second more than the minimum
  m_time_lord->setTypeTimeout(1000); // 0.5 second more than the minimum

  m_player->togglePlayPause(true);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);

  // Start typing within the adjusted type timer interval
  m_time_lord->keyTyped();
  for (int i = 0; i < 7; i++) {
    QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);
    QTest::qWait(450);
    m_time_lord->keyTyped();
  }

  // Wait timer should have run out
  QCOMPARE(m_player->getState(), AudioPlayer::WAITING);

  // Keep typing within the type timer interval
  for (int i = 0; i < 4; i++) {
    QTest::qWait(900);
    m_time_lord->keyTyped();
  }
  QCOMPARE(m_player->getState(), AudioPlayer::WAITING);

  // Stop typing
  QTest::qWait(1100);
  QCOMPARE(m_player->getState(), AudioPlayer::PLAYING);
}
