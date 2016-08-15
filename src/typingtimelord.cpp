#include "typingtimelord.h"

TypingTimeLord::TypingTimeLord(std::shared_ptr<AudioPlayer> player,
                               QObject* parent) :
  QObject(parent),
  m_player(player) {

  QObject::connect(m_player.get(), SIGNAL(stateChanged()),
                   this,           SLOT(playerStateChanged()));

  // Initialize the timers to single shot timers and connect them to their
  // respective fallbacks;
  m_wait_timer.setSingleShot(true);
  m_type_timer.setSingleShot(true);
  connect(&m_wait_timer, SIGNAL(timeout()), this, SLOT(waitTimeout()));
  connect(&m_type_timer, SIGNAL(timeout()), this, SLOT(typeTimeout()));
}

void TypingTimeLord::restartWaitTimer() {
  if (m_player->getState() == AudioPlayer::PLAYING) {
    m_wait_timer.stop();
    m_wait_timer.start(m_wait_timeout);
  }
}

void TypingTimeLord::restartTypeTimer() {
  if (m_player->getState() != AudioPlayer::PAUSED) {
    m_type_timer.stop();
    m_type_timer.start(m_type_timeout);
  }
}

void TypingTimeLord::keyTyped() {
  if (m_player->getState() == AudioPlayer::PLAYING) {
    if (!m_wait_timer.isActive()) {
      m_wait_timer.start(m_wait_timeout);
    }
  }
  restartTypeTimer();
}

void TypingTimeLord::waitTimeout() {
  m_wait_timer.stop();
  m_player->toggleWaiting(true);
}

void TypingTimeLord::typeTimeout() {
  if (m_player->getState() == AudioPlayer::WAITING) {
    m_player->toggleWaiting(false);
  } else if (m_player->getState() == AudioPlayer::PLAYING) {
    restartWaitTimer();
  }

  // We're a signal of absence (of typing), so we should keep generating
  // signals that we're still not typing (until we are overridden by typing,
  // of course)
  restartTypeTimer();
}

void TypingTimeLord::playerStateChanged() {
  if (m_player->getState() != AudioPlayer::PLAYING) {
    m_wait_timer.stop();
  }
  if (m_player->getState() == AudioPlayer::PAUSED) {
    m_type_timer.stop();
  }
}
