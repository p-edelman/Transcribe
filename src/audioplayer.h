#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>

#include <QAudioOutput>
#include <QAudioProbe>
#include <QDebug>
#include <QMediaPlayer>
#include <QString>

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

  PlayerState getState();
  uint getDuration();
  uint getPosition();

  /** Open a new audio file.
   *  @param path the complete path to the new file. */
  void openFile(const QString &path);

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

  /** Signals that the audio failed to load or play.
   *  @param message an error message that can be displayed to the user. */
  void error(const QString& message);

public slots:
  /** Seek to the specified position in the stream. */
  void setPosition(int seconds);

  /** Skip a number of seconds backward or forward in the audio stream. */
  void skipSeconds(AudioPlayer::SeekDirection direction, int seconds);

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

  /** The output channel we use to write raw audio data to. */
  QIODevice* m_playback_device = NULL;

  /** The audio format we're currently working with as a reference to see if
   *  something has changed. */
  QAudioFormat* m_audio_format;

  /** The factor that audio data is multiplied with before playing. */
  qreal m_boost = 1.0;

  /** Amplified the audio by the m_boost factor and store the result in
   *  m_modified_buffer. Check the return value to make sure the data in
   *  m_modified_buffer is valid!
   *  If the signal is boosted outside its bound, it will be clipped.
   *  @tparam word_type the type of the audio data.
   *  @param buffer the audio buffer.
   *  @return true if the signal was modified, false otherwise. If false is
                   returned, the data in m_modified_buffer is invalid! */
  template<class word_type> bool boostAudio(const QAudioBuffer& buffer);

  /** The main QMediaPlayer instance for playing and seeking audio files. */
  QMediaPlayer* m_player;

  /** We're using a bit of a dirty trick here to be able to modify the audio
   *  signal before actually playing it. Normally, one would use the
   *  QAudioDecoder class for this use case, but that doesn't allow seeking.
   *  Instead we set the volume of the QMediaPlayer to 0 and attach a
   *  QAudioProbe to the it, which hands out the raw audio data via the
   *  audioBufferProbed() signal, which we'll intercept with
   *  handleAudioBuffer(). This readonly data - QAudioProbe is meant to monitor
   *  the audio data, not to modify it, so we write the modified signal to a new
   *  buffer, m_modified buffer and play back that copy. */
  QAudioProbe* m_probe;

  /** Buffer for storing modified audio data. We declare it just once to avoid
   *  the overhead of repeated memory operations. */
  char*  m_modified_buffer = NULL;

  /** Remember the size of the modified audio data buffer, so that we can
   *  enlarge it if a new buffer arrives which requires more space. We never
   *  reduce the size though, as audio buffers are fairly constant in size and
   *  it wouldn't make much sense to reclaim the small amount of memory. */
  int m_modified_buffer_size = 0;

  /** To figure out by how much we can boost an audio sample without clipping it
   *  too much, we have to count how many of each sample point there are.
   *  We use int for this as the QAudioFormat::sampleCount() method uses this as
   *  the return type, thus the actual count fits into an int. */
  int* m_spectrogram = NULL;

  /** Remember the size of the spectrogram buffer, so we can enlarge it if a
   *  sample size is picked that uses larger values. */
  int m_spectrogram_size = 0;

  /** When the audio fails to load, oftentimes multiple error messages are
   *  thrown by QMediaPlayer. We need to signal a problem just once to the end
   *  user though, so we need to keep track of whether it is handled already. */
  bool m_error_handled = false;
};

#endif // AUDIOPLAYER_H
