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
                     this, SLOT(audioAvailableChanged(bool)));
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

void AudioPlayer::audioAvailableChanged(bool available) {
  // Set the time of the MediaControls to the new value or 0 if no media is
  // available.
  QVariant seconds;
  QVariant ret_val;
  if (available) {
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
