#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>

#include <QAudioOutput>
#include <QDebug>
#include <QString>

#include "sonicbooster.h"
#include "audiodecoder.h"

/** The 'back-end' class for playing audio files. It is complemented by a
 *  QML MediaControls element to interact with it. */
class AudioPlayer : public QObject {
  Q_OBJECT

public:
  explicit AudioPlayer(QObject* parent = 0);

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
  Q_PROPERTY(PlayerState state
             READ getState
             NOTIFY stateChanged)

  /** The duration of the loaded audio file in whole seconds. */
  Q_PROPERTY(uint duration
             READ getDuration
             NOTIFY durationChanged)

  /** The position in the audio playback in whole seconds. */
  Q_PROPERTY(uint position
             READ getPosition
             NOTIFY positionChanged)

  /** The ability to boost the audio. */
  Q_PROPERTY(bool can_boost
             READ canBoost
             NOTIFY canBoostChanged)

  /** Open a new audio file.
   *  @param path the complete path to the new file. */
  void openFile(const QString &path);

  /** Return the path of the loaded file. */
  QString getFilePath();

  PlayerState getState();
  uint getDuration();
  uint getPosition();

  /** Indicates if we can boost the current loaded audio *to the best of our
   *  current knowledge!* That is, we might think that we can boost the audio
   *  but it may turn out that we can't if we try. In that case, subsequent
   *  calls to this method return false. */
  bool canBoost();

signals:
  /** Signals the the playing state has changed. */
  void stateChanged();

  /** Signals that the duration of the audio has changed. This typically occurs
   *  when a new audio file is loaded. */
  void durationChanged();

  /** Signals that the position in the audio playback is changed. This usually
   *  occurs every millisecond, even though the position property is measured
   *  in whole seconds. */
  void positionChanged();

  /** Signals that the ability to boost the audio has changed. */
  void canBoostChanged();

  /** Signals that the audio failed to load or play.
   *  @param message an error message that can be displayed to the user. */
  void error(const QString& message);

public slots:
  /** Seek to the specified position in the stream. */
  void setPosition(int seconds);

  /** Skip a number of seconds in the audio stream
   *  @param seconds the number of seconds to skip, either negative or positve.
   */
  void skipSeconds(int seconds);

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

  /** Increase or decrease the boost factor of the audio by 0.1.
   *  @param is_up if true, the boost factor is increased, if false it is
   *               decreased. */
  void boost(bool is_up);

private slots:
  /** Callback for when QMediaPlayer reports that the position in the media
   *  stream has changed. */
  void handleMediaPositionChanged(qint64);

  /** Callback for when the QMediaPlayer has finished loading a new audio file
   *  or the duration is changed. */
  void handleMediaAvailabilityChanged();

  /** Callback for when an error occurs during media loading by the
   *  QMediaPlayer. */
  void handleMediaError();

  /** Callback for when the QMediaPlayer reports changes in the media status.
   *  Needed to catch the end of audio situation. */
  void handleMediaStatusChanged(QMediaPlayer::MediaStatus status);

  /** Callback for when the QAudioProbe received a new buffer. It will make
      play back this buffer, possibly altered, to the m_playback_device. */
  void handleAudioBuffer(const QAudioBuffer& buffer);

private:
  /** Set the PlayerState to the desired state. In response, the appriate
   *  signals will be sent.
   *  @param state the desired state */
  void setState(PlayerState state);

  /** The state that we're currently in. */
  PlayerState m_state;

  /** Initialize the audio device to which the audio data will be sent for the
   *  specific audio format. It needs to be called anytime the the audioformat
   *  changes, which is dictated by the QMediaPlayer but generally only happens
   *  when a new audio file is loaded. */
  void initAudioDevice(const QAudioFormat& format);

  /** The main AudioDecoder instance for playing and seeking audio files. */
  AudioDecoder m_decoder;

  /** The SonicBooster instance for amplifying the audio signal. */
  SonicBooster m_sonic_booster;

  /** When the audio fails to load, oftentimes multiple error messages are
   *  thrown by QMediaPlayer. We need to signal a problem just once to the end
   *  user though, so we need to keep track of whether it is handled already. */
  bool m_error_handled = false;

  /** Remember if we can boost the audio. We can get an indication for this from
   *  the AudioDecoded, but we can't know for sure until we actually try it. */
  bool m_can_boost = false;

  /** Message to display to the user if he tries to boost the audio, but this
   *  isn't possible.
   *  Unfortunately this check is somewhat complicated, because it depends on
   *  two factors; one, is the AudioDecoder sending raw audio data (vs. playing
   *  directly), and two, can the SonicBooster amplify this format. The second
   *  question can only be answered when we're receiving a QAudioBuffer, but
   *  this doesn't happen at all if the first condition isn't met. So we're
   *  checking in two places: the boost() method when the user adjusts the
   *  boost factor for the first condition, and the handleAudioBuffer() method
   *  for the second factor. The we can use this message to report the error. */
#ifdef Q_OS_ANDROID
  const QString BOOST_UNSUPPORTED_MSG = tr("Sorry, but only .wav files can be amplified.");
#else
  const QString BOOST_UNSUPPORTED_MSG = tr("Sorry, but this audio format can't be amplified.");
#endif
};

#endif // AUDIOPLAYER_H
