#include "audioplayer.h"

AudioPlayer::AudioPlayer(QObject *parent) : QObject(parent) {
  m_player   = new QMediaPlayer;
  m_controls = NULL;
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
    QObject::connect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)),
                     this, SLOT(playingStateChanged(QMediaPlayer::State)));
  }
}

void AudioPlayer::openAudioFile(const QString &url) {
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

void AudioPlayer::togglePlayPause(bool should_be_playing) {
  if (m_player->state() == QMediaPlayer::PlayingState) {
    if (!should_be_playing) {
      m_player->pause();
    }
  } else {
    if (should_be_playing) {
      m_player->play();
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

void AudioPlayer::playingStateChanged(QMediaPlayer::State state) {
  // Signal the Controls that the audio has started or stopped playing.
  QVariant is_playing;
  if (state == QMediaPlayer::PlayingState) {
    is_playing.setValue(true);
  } else {
    is_playing.setValue(false);
  }

  QVariant ret_val;
  QMetaObject::invokeMethod(m_controls, "setPlayingState",
                            Q_RETURN_ARG(QVariant, ret_val),
                            Q_ARG(QVariant, is_playing));
}

