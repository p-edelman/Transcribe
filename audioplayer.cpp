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
  }
}

void AudioPlayer::openAudioFile(const QString &url) {
  m_player->stop();
  m_player->setMedia(QUrl(url));
}

void AudioPlayer::audioAvailableChanged(bool available) {
  if (available) {
    // Set the end time of the MediaControls.
    QVariant seconds(round(m_player->duration() / 1000));
    QVariant ret_val;
    QMetaObject::invokeMethod(m_controls, "setDuration",
                              Q_RETURN_ARG(QVariant, ret_val),
                              Q_ARG(QVariant, seconds));
  }
  m_player->play();
}

void AudioPlayer::setAudioPosition(int seconds) {
  m_player->setPosition(seconds * 1000);
}

void AudioPlayer::audioPositionChanged(qint64 milliseconds) {
  // Set the AudioControls slider to the new position
  QVariant seconds(round(milliseconds / 1000));
  QVariant ret_val;
  QMetaObject::invokeMethod(m_controls, "setPosition",
                            Q_RETURN_ARG(QVariant, ret_val),
                            Q_ARG(QVariant, seconds));
}
