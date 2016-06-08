#include "audioplayer.h"

AudioPlayer::AudioPlayer(QObject *parent) : QObject(parent) {
  m_state    = PlayerState::PAUSED;

  m_player   = new QMediaPlayer;
  m_controls = NULL;

  // Initialize the timers to single shot timers and connect them to their
  // respective fallbacks;
  m_pause_timer  = new QTimer();
  m_typing_timer = new QTimer();
  m_pause_timer->setSingleShot(true);
  m_typing_timer->setSingleShot(true);
  connect(m_pause_timer, SIGNAL(timeout()), this, SLOT(pauseTimeout()));
  connect(m_typing_timer, SIGNAL(timeout()), this, SLOT(typingTimeout()));
}

void AudioPlayer::setAudioControls(QObject* controls) {
  if (controls != NULL) {
    m_controls = controls;

    // Connect the signals of QMediaPlayer and MediaControls to the callback
    // methods.
    QObject::connect(m_controls, SIGNAL(valueChanged(int)),
                     this, SLOT(setAudioPosition(int)));
    QObject::connect(m_player, SIGNAL(positionChanged(qint64)),
                     this, SLOT(audioPositionChanged(qint64)));
    QObject::connect(m_player, SIGNAL(audioAvailableChanged(bool)),
                     this, SLOT(audioAvailabilityChanged()));
    QObject::connect(m_player, SIGNAL(durationChanged(qint64)),
                     this, SLOT(audioAvailabilityChanged()));
    QObject::connect(m_controls, SIGNAL(playingStateChanged(bool)),
                     this, SLOT(togglePlayPause(bool)));
  }
}

void AudioPlayer::seek(SeekDirection direction, int seconds) {
  if (direction == SeekDirection::FORWARD) {
    m_player->setPosition(m_player->position() + seconds * 1000);
  } else {
    m_player->setPosition(m_player->position() - seconds * 1000);
  }
}

void AudioPlayer::openAudioFile(const QUrl& url) {
  m_player->stop();
  m_player->setMedia(QUrl(url));
}

void AudioPlayer::audioAvailabilityChanged() {
  // Set the duration of the MediaControls to the duration of the loaded
  // media if it is availabe, or 0 if no media is available.
  // Note thatthe reported duration might be -1 initially even though the
  // audio is available. That's why this method needs to be bound to the
  // durationChanged signal as well.
  QVariant seconds;
  QVariant ret_val;
  if (m_player->isAudioAvailable()) {
    seconds.setValue(round(m_player->duration() / 1000));
  } else {
    seconds.setValue(0);
  }
  QMetaObject::invokeMethod(m_controls, "setDuration",
                            Q_RETURN_ARG(QVariant, ret_val),
                            Q_ARG(QVariant, seconds));
}

void AudioPlayer::setAudioPosition(int seconds) {
  m_player->setPosition(seconds * 1000);
}

void AudioPlayer::setState(PlayerState state) {
  if (state != m_state) {
    m_state = state;

    // Take care of the audio and timers
    if (m_state == PlayerState::PAUSED) {
      m_pause_timer->stop();
      m_typing_timer->stop();
      m_player->pause();
    } else if (m_state == PlayerState::PLAYING) {
      m_player->play();
    } else if (m_state == PlayerState::WAITING) {
      m_pause_timer->stop();
      m_player->pause();
    }

    // Signal the Controls that the audio has started or stopped playing.
    QVariant is_playing(m_state);
    QVariant ret_val;
    QMetaObject::invokeMethod(m_controls, "setState",
                              Q_RETURN_ARG(QVariant, ret_val),
                              Q_ARG(QVariant, is_playing));
  }
}

void AudioPlayer::togglePlayPause() {
  if (m_state == PlayerState::PAUSED) {
    setState(PlayerState::PLAYING);
  } else {
    // If either PLAYING or WAITING, go to PAUSED
    setState(PlayerState::PAUSED);
  }
}

void AudioPlayer::togglePlayPause(bool should_play) {
  if (m_state == PlayerState::PAUSED) {
    if (should_play) {
      // We're toggling between PAUSED and PLAYING here, so if we're WAITING,
      // we should remain there.
      setState(PlayerState::PLAYING);
    }
  } else {
    // We can go to PAUSED from both PLAYING and WAITING state
    if (m_state != PlayerState::PAUSED) {
      setState(PlayerState::PAUSED);
    }
  }
}

void AudioPlayer::toggleWaiting(bool should_wait) {
  if (should_wait) {
    if (m_state == PlayerState::PLAYING) {
      setState(PlayerState::WAITING);
    }
  } else {
    if (m_state == PlayerState::WAITING) {
      setState(PlayerState::PLAYING);
    }
  }
}

void AudioPlayer::audioPositionChanged(qint64 milliseconds) {
  // Set the AudioControls slider to the new position
  QVariant seconds(round(milliseconds / 1000));
  QVariant ret_val;
  QMetaObject::invokeMethod(m_controls, "setPosition",
                            Q_RETURN_ARG(QVariant, ret_val),
                            Q_ARG(QVariant, seconds));
}

void AudioPlayer::restartPauseTimer() {
  if (m_state == PlayerState::PLAYING) {
    m_pause_timer->stop();
    m_pause_timer->start(m_pause_timeout);
  }
}

void AudioPlayer::restartTypingTimer() {
  if (m_state != PlayerState::PAUSED) {
    m_typing_timer->stop();
    m_typing_timer->start(m_typing_timeout);
  }
}

void AudioPlayer::maybeStartPauseTimer() {
  if (m_state == PlayerState::PLAYING) {
    if (!m_pause_timer->isActive()) {
      m_pause_timer->start(m_pause_timeout);
    }
  }
}

void AudioPlayer::pauseTimeout() {
  toggleWaiting(true);
}

void AudioPlayer::typingTimeout() {
  if (m_state == PlayerState::WAITING) {
    setState(PlayerState::PLAYING);
  } else if (m_state == PlayerState::PLAYING) {
    restartPauseTimer();
  }

  // We're a signal of absence (of typing), so we should keep generating
  // signals that we're still not typing (untill we are overridden by typing,
  // of course)
  restartTypingTimer();
}


