#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>

#include <QDebug>
#include <QMediaPlayer>

/** The 'back-end' class for playing audio files. It is complemented by a
 *  QML MediaControls element to interact with it. */
class AudioPlayer : public QObject {
  Q_OBJECT

public:
  explicit AudioPlayer(QObject *parent = 0);

  /** Associate a QML MediaControls element for interacting with the audio
   *  playback.
   *  @param controls a fully initialized QML MediaControls element. */
  void setAudioControls(QObject* controls);

signals:

public slots:
  /** Open a new audio file.
   *  @param url an as QUrl formatted QString of the new file location. */
  void openAudioFile(const QString &url);

  /** Seek to the specified position in the stream. */
  void setAudioPosition(int seconds);

  /** Callback for when the position in the audio stream has changed to update
   *  the AudioControls element. */
  void audioPositionChanged(qint64 milliseconds);

  /** Callback for when the QMediaPlayerhas finished loading a new audio file.
    */
  void audioAvailableChanged(bool available);

private:
  /** The main QMediaPlayer instance for playing and seeking audio files. */
  QMediaPlayer* m_player;

  /** The visual controls. */
  QObject* m_controls;
};

#endif // AUDIOPLAYER_H