#include "audioplayer.h"

AudioPlayer::AudioPlayer(QObject *parent) : QObject(parent) {
  m_state = PlayerState::PAUSED;

  m_audio_format = new QAudioFormat();

  m_player = new QMediaPlayer;
  m_player->setNotifyInterval(1000); // We're working with second precision
  QObject::connect(m_player, SIGNAL(positionChanged(qint64)),
                   this, SLOT(handleMediaPositionChanged(qint64)));
  QObject::connect(m_player, SIGNAL(audioAvailableChanged(bool)),
                   this, SLOT(handleMediaAvailabilityChanged()));
  QObject::connect(m_player, SIGNAL(durationChanged(qint64)),
                   this, SLOT(handleMediaAvailabilityChanged()));
  QObject::connect(m_player, SIGNAL(error(QMediaPlayer::Error)),
                   this, SLOT(handleMediaError()));
  QObject::connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
                   this, SLOT(handleMediaStatusChanged(QMediaPlayer::MediaStatus)));

  // Let the QAudioProbe 'intercept' the audio data.
  m_player->setVolume(0);
  m_probe = new QAudioProbe(parent);
  m_probe->setSource(m_player);

  connect(m_probe, SIGNAL(audioBufferProbed(QAudioBuffer)),
          this, SLOT(handleAudioBuffer(QAudioBuffer)));
}

AudioPlayer::PlayerState AudioPlayer::getState() {
  return m_state;
}

uint AudioPlayer::getDuration() {
  if (m_player->duration() > 0) {
    return ((m_player->duration() + 500) / 1000);
  }
  return 0;
}

uint AudioPlayer::getPosition() {
  return ((m_player->position() + 500) / 1000);
}

void AudioPlayer::skipSeconds(SeekDirection direction, int seconds) {
  qint64 new_pos;
  if (direction == SeekDirection::FORWARD) {
    new_pos = m_player->position() + seconds * 1000;
    if (new_pos > m_player->duration()) {
      new_pos = m_player->duration();
    }
  } else {
    new_pos = m_player->position() - seconds * 1000;
    if (new_pos < 0) {
      new_pos = 0;
    }
  }
  m_player->setPosition(new_pos);
  emit positionChanged();
}

void AudioPlayer::openFile(const QString& path) {
  m_error_handled = false;

  setState(PlayerState::PAUSED);
  m_player->setMedia(QUrl::fromLocalFile(path));
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
    message += m_player->errorString();

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
  if (ms > m_player->duration()) ms = m_player->duration(); // Cap
  m_player->setPosition(ms);
}

void AudioPlayer::setState(PlayerState state) {
  // If the media is not properly loaded or we're at the end of the media, we
  // can only be in the paused state.
  if ((m_player->mediaStatus() != QMediaPlayer::LoadedMedia) &&
      (m_player->mediaStatus() != QMediaPlayer::BufferedMedia)) {
    state = PlayerState::PAUSED;
  }

  if (state != m_state) {
    m_state = state;

    // Take care of the audio
    if (m_state == PlayerState::PAUSED) {
      // Corner case: if we're at the end of the media, don't call pause()
      // because it will reset the audio to the start.
      if (m_player->mediaStatus() != QMediaPlayer::EndOfMedia) {
        m_player->pause();
      }
    } else if (m_state == PlayerState::PLAYING) {
      m_player->play();
    } else if (m_state == PlayerState::WAITING) {
      m_player->pause();
    }

    emit stateChanged();
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
  if (is_up) {
    m_boost += 0.1;
  } else {
    m_boost -= 0.1;
    if (m_boost < 0.0) m_boost = 0.0;
  }
}

void AudioPlayer::handleMediaPositionChanged(qint64) {
  emit positionChanged();
}

template<class word_type>
bool AudioPlayer::boostAudio(const QAudioBuffer& buffer) {
  if (m_boost != 1.0) {
    const word_type* data = buffer.constData<word_type>();
    word_type* new_buffer = (word_type*)m_modified_buffer;
    for (int i = 0; i < buffer.frameCount(); i++) {
      // qint32 should be sufficient as we only handle 8 and 16 bit data.
      qint32 val = (qint32)(data[i] * m_boost);

      // Cap the value if needed
      if (val > std::numeric_limits<word_type>::max()) {
        new_buffer[i] = std::numeric_limits<word_type>::max();
      } else if (val < std::numeric_limits<word_type>::min()) {
        new_buffer[i] = std::numeric_limits<word_type>::min();
      } else {
        new_buffer[i] = (word_type)val;
      }
    }
    return true;
  }

  return false;
}

void AudioPlayer::initAudioDevice(const QAudioFormat& format) {
  if (format != *m_audio_format) {
    qDebug() << "Change of format";
    m_audio_format = new QAudioFormat(format);
    delete m_playback_device; m_playback_device = NULL;
  }

  if (m_playback_device == NULL) {
    QAudioOutput* device = new QAudioOutput(format, this);
    device->setVolume(1.0);
    m_playback_device = device->start();
  }
}

void AudioPlayer::handleAudioBuffer(const QAudioBuffer& buffer) {
  initAudioDevice(buffer.format());

  if (buffer.isValid()) {
    if (buffer.byteCount() > m_modified_buffer_size) {
      m_modified_buffer_size = buffer.byteCount();
      m_modified_buffer  = (char*)realloc(m_modified_buffer, m_modified_buffer_size);
    }

    bool is_modified = false;

    if (buffer.format().sampleType() == QAudioFormat::SignedInt) {
      if (buffer.format().bytesPerFrame() == 1) {
        is_modified = boostAudio<int8_t>(buffer);
      } else if (buffer.format().bytesPerFrame() == 2) {
        is_modified = boostAudio<int16_t>(buffer);
      }
    } else {
      // TODO: Unreadable
      qDebug() << "Unsupported format";
    }

    // Finally, play the audio
    if (is_modified) {
      m_playback_device->write(m_modified_buffer, buffer.byteCount());
    } else {
      m_playback_device->write((char*)buffer.constData(), buffer.byteCount());
    }
  }
}
