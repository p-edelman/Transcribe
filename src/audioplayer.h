#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>

#include <QDebug>
#include <QMediaPlayer>
#include <QString>
#include <QTimer>

enum SeekDirection {FORWARD, BACKWARD};

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

  /** Associate a QML MediaControls element for interacting with the audio
   *  playback.
   *  @param controls a fully initialized QML MediaControls element. */
  void setAudioControls(QObject* controls);

  /** Open a new audio file.
   *  @param path the complete path to the new file. */
  void openAudioFile(const QString &path);

  /** Skip a number of seconds backward or forward in the audio stream. */
  void seek(SeekDirection direction, int seconds);

  /** Cancel the pause timer if it is running and start it from the beginning.
   *  This will be ignored if the player is not in PLAYING state. */
  void restartPauseTimer();

  /** Cancel the typing timer if it is running and start it from the beginning.
   *  This will be ignored if the player is not in PLAYING of WAITING state. */
  void restartTypingTimer();

  /** Start the pause timer if it is not running and if the audio is in PLAYING
   *  state. */
  void maybeStartPauseTimer();

signals:
  /** Signals the the playing state has changed. */
  void playerStateChanged(PlayerState state);

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

  /** Callback for when the position in the audio stream has changed. */
  void audioPositionChanged(qint64);

  /** Callback for when the QMediaPlayer has finished loading a new audio file
   *  or the duration is changed. */
  void audioAvailabilityChanged();

  /** Callback for when an error occurs during media loading by the
   *  QMediaPlayer. */
  void handleMediaError();

private slots:
  /** Callback for when the pause timer expires. */
  void pauseTimeout();

  /** Callback for when the typing timer expires. */
  void typingTimeout();

private:
  /** Set the PlayerState to the desired state and update the relevant timers
   *  and GUI.
   *  @param state the desired state */
  void setState(PlayerState state);

  /** The state that we're currently in. */
  PlayerState m_state;

  /** The main QMediaPlayer instance for playing and seeking audio files. */
  QMediaPlayer* m_player;

  /** The visual controls. */
  QObject* m_controls;

  /** We keep two timers to determine if we should wait the audio playback:
   *  - m_pause_timer  lets the audio run for just some amount of time
   *  - m_typing_timer monitors if the user stops typing
   *
   * The idea is that when the user keeps typing, the playback should wait until
   * she stops typing before continuing. This can be achieved with the following
   * set of conditions:
   *
   * When the user types a key (other than audio keys):
   * - When PLAYING: if the pausing timer is not running, start it
   * - When PLAYING or PAUSING: (re)start the typing timer
   *
   * On pausing timer expiration (the audio has run for some time and the user
   * is still typing):
   * - When PLAYING: switch to WAITING state and stop the pausing timer
   *
   * On typing timer expiration (the user has stopped typing):
   * - When PLAYING: reset the pausing timer
   * - When WAITING: switch to PLAYING state
   *
   * On switching to PAUSED state: stop both timers
   */
  QTimer* m_pause_timer;
  QTimer* m_typing_timer;

  /** The intervals for the two timers. */
  unsigned int m_pause_timeout  = 5000;
  unsigned int m_typing_timeout = 1000;

  /** When the audio fails to load, oftentimes multiple error messages are
   *  thrown by QMediaPlayer. We need to signal a problem just once to the end
   *  user though, so we need to keep track of whether it is handled already. */
  bool m_error_handled = false;
};

#endif // AUDIOPLAYER_H
