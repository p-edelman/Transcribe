#include "audioplayer.h"

AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent) {

  m_state = PlayerState::PAUSED;

  m_decoder.setNotifyInterval(1000); // We're working with second precision
  connect(&m_decoder, SIGNAL(positionChanged(qint64)),
          this,       SLOT(handleMediaPositionChanged(qint64)));
  connect(&m_decoder, SIGNAL(durationChanged(qint64)),
          this,       SLOT(handleMediaAvailabilityChanged()));
  connect(&m_decoder, SIGNAL(audioAvailableChanged(bool)),
          this,       SLOT(handleMediaAvailabilityChanged()));
  connect(&m_decoder, SIGNAL(error(QMediaPlayer::Error)),
          this,       SLOT(handleMediaError()));
  connect(&m_decoder, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
          this,       SLOT(handleMediaStatusChanged(QMediaPlayer::MediaStatus)));
  connect(&m_decoder, SIGNAL(bufferReady(QAudioBuffer)),
          this,       SLOT(handleAudioBuffer(QAudioBuffer)));
}

void AudioPlayer::openFile(const QString& path) {
  m_sonic_booster.resetLevel();

  m_error_handled = false;

  setState(PlayerState::PAUSED);
  m_decoder.setMedia(QUrl::fromLocalFile(path));
}

QString AudioPlayer::getFilePath() {
  return m_decoder.getMediaPath();
}

AudioPlayer::PlayerState AudioPlayer::getState() {
  return m_state;
}

uint AudioPlayer::getDuration() {
  if (m_decoder.duration() > 0) {
    return ((m_decoder.duration() + 500) / 1000);
  }
  return 0;
}

uint AudioPlayer::getPosition() {
  return ((m_decoder.position() + 500) / 1000);
}

void AudioPlayer::skipSeconds(SeekDirection direction, int seconds) {
  qint64 new_pos;
  if (direction == SeekDirection::FORWARD) {
    new_pos = m_decoder.position() + seconds * 1000;
    if (new_pos > m_decoder.duration()) {
      new_pos = m_decoder.duration();
    }
  } else {
    new_pos = m_decoder.position() - seconds * 1000;
    if (new_pos < 0) {
      new_pos = 0;
    }
  }
  m_decoder.setPosition(new_pos);
  emit positionChanged();
}

void AudioPlayer::handleMediaAvailabilityChanged() {
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
    message += m_decoder.errorString();

    emit error(message);
  }
}

void AudioPlayer::handleMediaStatusChanged(QMediaPlayer::MediaStatus status) {
  if (status == QMediaPlayer::EndOfMedia) {
    // When the media ends, switch to the PAUSED state.
    setState(PlayerState::PAUSED);
  }
}

void AudioPlayer::setPosition(int seconds) {
  qint64 ms = seconds * 1000;
  if (ms > m_decoder.duration()) ms = m_decoder.duration(); // Cap
  m_decoder.setPosition(ms);
}

void AudioPlayer::setState(PlayerState state) {
  // If the media is not properly loaded or we're at the end of the media, we
  // can only be in the paused state.
  if ((m_decoder.mediaStatus() != QMediaPlayer::LoadedMedia) &&
      (m_decoder.mediaStatus() != QMediaPlayer::BufferedMedia)) {
    state = PlayerState::PAUSED;
  }

  if (state != m_state) {
    m_state = state;
    emit stateChanged();

    // Take care of the audio
    if (m_state == PlayerState::PAUSED) {
      // Corner case: if we're at the end of the media, don't call pause()
      // because it will reset the audio to the start.
      if (m_decoder.mediaStatus() != QMediaPlayer::EndOfMedia) {
        m_decoder.pause();
      }
    } else if (m_state == PlayerState::PLAYING) {
      m_decoder.play();
    } else if (m_state == PlayerState::WAITING) {
      m_decoder.pause();
    }
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

void AudioPlayer::boost(bool is_up) {
  if (m_decoder.isIntercepting()) {
    if (is_up) {
      m_sonic_booster.increaseLevel();
    } else {
      m_sonic_booster.decreaseLevel();
    }
  } else {
    emit error(BOOST_UNSUPPORTED_MSG);
  }
}

void AudioPlayer::handleMediaPositionChanged(qint64) {
  emit positionChanged();
}

void AudioPlayer::handleAudioBuffer(const QAudioBuffer& buffer) {
  if (buffer.isValid()) {
    if (m_sonic_booster.level() != 0) {
      if (!m_sonic_booster.canBoost(buffer.format())) {
        emit error(BOOST_UNSUPPORTED_MSG);
        m_sonic_booster.resetLevel();
      }
    }
    bool is_modified = m_sonic_booster.boost(buffer);

    // Finally, play the audio
    if (is_modified) {
      int         boosted_buffer_size;
      const char* boosted_buffer = m_sonic_booster.getBoostedBuffer(boosted_buffer_size);
      m_decoder.playbackDevice()->write(boosted_buffer, boosted_buffer_size);
    } else {
      m_decoder.playbackDevice()->write((char*)buffer.constData(), buffer.byteCount());
    }
  }
}
