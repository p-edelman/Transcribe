#include "audioplayer.h"

AudioPlayer::AudioPlayer(QObject *parent) : QObject(parent) {
  m_state = PlayerState::PAUSED;

  m_player = new QMediaPlayer;
  QObject::connect(m_player, SIGNAL(positionChanged(qint64)),
                   this, SLOT(audioPositionChanged(qint64)));
  QObject::connect(m_player, SIGNAL(audioAvailableChanged(bool)),
                   this, SLOT(audioAvailabilityChanged()));
  QObject::connect(m_player, SIGNAL(durationChanged(qint64)),
                   this, SLOT(audioAvailabilityChanged()));
  QObject::connect(m_player, SIGNAL(error(QMediaPlayer::Error)),
                   this, SLOT(handleMediaError()));
  QObject::connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
                   this, SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));

  // Initialize the timers to single shot timers and connect them to their
  // respective fallbacks;
  m_pause_timer  = new QTimer();
  m_typing_timer = new QTimer();
  m_pause_timer->setSingleShot(true);
  m_typing_timer->setSingleShot(true);
  connect(m_pause_timer, SIGNAL(timeout()), this, SLOT(pauseTimeout()));
  connect(m_typing_timer, SIGNAL(timeout()), this, SLOT(typingTimeout()));
}

AudioPlayer::PlayerState AudioPlayer::getPlayerState() {
  return m_state;
}

uint AudioPlayer::getDuration() {
  if (m_player->duration() > 0) {
    return (m_player->duration() / 1000);
  }
  return 0;
}

uint AudioPlayer::getPosition() {
  return (m_player->position() / 1000);
}

void AudioPlayer::seek(SeekDirection direction, int seconds) {
  if (direction == SeekDirection::FORWARD) {
    m_player->setPosition(m_player->position() + seconds * 1000);
  } else {
    m_player->setPosition(m_player->position() - seconds * 1000);
  }
}

void AudioPlayer::openAudioFile(const QString& path) {
  m_error_handled = false;

  m_player->stop();
  m_player->setMedia(QUrl::fromLocalFile(path));
}

void AudioPlayer::audioAvailabilityChanged() {
  // Signal the MediaControls that the duration of the loaded media has changed.
  // Note that the reported duration might be -1 initially even though the
  // audio is available. That's why this method needs to be bound to the
  // durationChanged signal as well.
  emit durationChanged();
}

void AudioPlayer::handleMediaError() {
  if (!m_error_handled) {
    m_error_handled = true;

    QString message = tr("The audio file can't be loaded.\n");
    message += m_player->errorString();

    emit audioError(message);
  }
}

void AudioPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status) {
  if (status == QMediaPlayer::EndOfMedia) {
    // When the media ends, switch to the PAUSED state.
    setState(PlayerState::PAUSED);
  }
}

void AudioPlayer::setAudioPosition(int seconds) {
  m_player->setPosition(seconds * 1000);
}

void AudioPlayer::setState(PlayerState state) {
  // Corner case: if we're at the end of the media, we can only be in paused
  // state.
  if (m_player->mediaStatus() == QMediaPlayer::EndOfMedia) {
    state = PlayerState::PAUSED;
  }

  if (state != m_state) {
    m_state = state;

    // Take care of the audio and timers
    if (m_state == PlayerState::PAUSED) {
      m_pause_timer->stop();
      m_typing_timer->stop();
      // Corner case: if we're at the end of the media, don't call pause()
      // because it will reset the audio to the start.
      if (m_player->mediaStatus() != QMediaPlayer::EndOfMedia) {
        m_player->pause();
      }
    } else if (m_state == PlayerState::PLAYING) {
      m_player->play();
    } else if (m_state == PlayerState::WAITING) {
      m_pause_timer->stop();
      m_player->pause();
    }

    emit playerStateChanged();
  }
}

void AudioPlayer::togglePlayPause() {
  if (m_state == PlayerState::PAUSED) {
    togglePlayPause(true);
  } else {
    // If either PLAYING or WAITING, go to PAUSED
    togglePlayPause(false);
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
    if (!should_play) {
      // We can go to PAUSED from both PLAYING and WAITING state
      if (m_state != PlayerState::PAUSED) {
        setState(PlayerState::PAUSED);
      }
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

void AudioPlayer::audioPositionChanged(qint64) {
  emit positionChanged();
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


