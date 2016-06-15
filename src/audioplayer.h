#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>

#include <QDebug>
#include <QMediaPlayer>
#include <QString>
#include <QTimer>

/** The 'back-end' class for playing audio files. It is complemented by a
 *  QML MediaControls element to interact with it. */
class AudioPlayer : public QObject {
  Q_OBJECT

public:
  explicit AudioPlayer(QObject *parent = 0);

  /** The player can be in one of three states:
   *  - PLAYING  means that the audio is playing; sound comes out of the speaker
   *  - PAUSED   means that the audio is not playing; no sound comes out of the
   *             speaker (it means the same as stopped)
   *  - WAITING  means that the audio is in principle playing, but that the user
   *             has not caught up with the typing; so no audio comes out the
   *             speaker, but the audio isn't paused either.
   * The following transitions are possible:
   * - PLAYING -> WAITING
   *           -> PAUSED
   * - PAUSED  -> PLAYING
   * - WAITING -> PLAYING
   *           -> PAUSED
   */
  enum PlayerState {PLAYING, PAUSED, WAITING};
  Q_ENUMS(PlayerState)

  enum SeekDirection {FORWARD, BACKWARD};
  Q_ENUMS(SeekDirection)

  /** The player state. */
  Q_PROPERTY(PlayerState player_state
             READ getPlayerState
             NOTIFY playerStateChanged)

  /** The duration of the loaded audio file in whole seconds. */
  Q_PROPERTY(uint duration
             READ getDuration
             NOTIFY durationChanged)

  /** The position in the audio playback in whole seconds. */
  Q_PROPERTY(uint position
             READ getPosition
             NOTIFY positionChanged)

  PlayerState getPlayerState();
  uint getDuration();
  uint getPosition();

  /** Open a new audio file.
   *  @param path the complete path to the new file. */
  void openAudioFile(const QString &path);

signals:
  /** Signals the the playing state has changed. */
  void playerStateChanged();

  /** Signals that the duration of the audio has changed. This typically occurs
   *  when a new audio file is loaded. */
  void durationChanged();

  /** Signals that the position in the audio playback is changed. This usually
   *  occurs every millisecond, even though the position property is measured
   *  in whole seconds. */
  void positionChanged();

  /** Signals that the audio failed to load or play.
   *  @param message an error message that can be displayed to the user. */
  void audioError(const QString& message);

public slots:
  /** Seek to the specified position in the stream. */
  void setAudioPosition(int seconds);

  /** Skip a number of seconds backward or forward in the audio stream. */
  void seek(AudioPlayer::SeekDirection direction, int seconds);

  /** Switch between paused and playing states, depending on the current state:
   *  - if PLAYING of WAITING, switch to PAUSED
   *  - if PAUSED, switch to PLAYING
   */
  void togglePlayPause();

  /** Switch between paused and playing states, depending on the parameter:
   *  - if PAUSED we can switch to PLAYING (if WAITING, we can't switch to
   *    PLAYING)
   *  - if PLAYING or WAITING, we can switch to PAUSED
    * @param should_play indicates whether the audio should be playing or be
    *                    paused. */
  void togglePlayPause(bool should_play);

  /** Set the WAITING state
   *  - if PLAYING we can switch to WAITING
   *  - if WAYTING we can switch to PLAYING
   *  All other transitions are ignored.
   *  @param should_wait indicates whether we should wait. */
  void toggleWaiting(bool should_wait);

private slots:
  /** Callback for when the position in the audio stream has changed. */
  void audioPositionChanged(qint64);

  /** Callback for when the QMediaPlayer has finished loading a new audio file
   *  or the duration is changed. */
  void audioAvailabilityChanged();

  /** Callback for when an error occurs during media loading by the
   *  QMediaPlayer. */
  void handleMediaError();

  /** Callback for when the media status changes. Needed to catch the end of
   *  audio situation. */
  void mediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
  /** Set the PlayerState to the desired state and update the relevant timers
   *  and GUI.
   *  @param state the desired state */
  void setState(PlayerState state);

  /** The state that we're currently in. */
  PlayerState m_state;

  /** The main QMediaPlayer instance for playing and seeking audio files. */
  QMediaPlayer* m_player;

  /** When the audio fails to load, oftentimes multiple error messages are
   *  thrown by QMediaPlayer. We need to signal a problem just once to the end
   *  user though, so we need to keep track of whether it is handled already. */
  bool m_error_handled = false;
};

#endif // AUDIOPLAYER_H
