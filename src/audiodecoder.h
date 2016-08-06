#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <QMediaPlayer>

#include <QDebug>

#include <QAudioOutput>
#include <QAudioBuffer>
#include <QAudioFormat>
#include <QAudioProbe>
#include <QFile>
#include <QUrl>
#include <QtEndian>

/** A QMediaPlayer extension that is meant to sent out raw audio data so that
 *  the audio can be manipulated before playing. When this is not possible, this
 *  class acts as a normal QMediaPlayer.
 *  Users of this class should subscribe to the bufferReady() signal, which
 *  sends audio data as a QAudioBuffer. When you're done manipulating it, you
 *  can write the data to the audiodevice obtained by playbackDevice().
 *  If data cannot be intercepted, audio is played directly.
 *
 *  To intercept the data, we use a bit of a dirty trick. Normally, one would
 *  use the QAudioDecoder class for this use case, but that doesn't allow
 *  seeking.
 *  Instead we set the volume of the QMediaPlayer to 0 and attach a QAudioProbe
 *  to it, which hands out readonly data - QAudioProbe is meant to monitor the
 *  audio data, not to modify it. We resend that data in the bufferReady()
 *  signal, where a copy of it can be modified before playback.
 *
 *  On Android, neither QAudioDecoder nor QAudioProbe are supported. To have at
 *  least some form of modifyable audio, this class adds the possibility to
 *  play .wav files natively. */
class AudioDecoder : public QMediaPlayer {
  Q_OBJECT

public:
  explicit AudioDecoder(QObject* parent = 0);
  ~AudioDecoder();

  qint64 duration() const;
  qint64 position() const;

  MediaStatus mediaStatus() const;

  /** Return an opened QIODevice where raw audio data can be written to to play
   *  it back. */
  QIODevice* playbackDevice() {return m_audio_out_device;}

public slots:
  /** Load the specified file. This method returns immediately, but it sends out
   *  the durationChanged() and mediaStatusChanged() signals on success, or the
   *  error() signal on failure.
   *  TODO: error signal. */
  void setMedia(const QUrl& path);

  void pause();
  void play();
  void setPosition(qint64 position);

signals:
  /** Connect to this signal to receive the raw audio data. */
  void bufferReady(const QAudioBuffer& buffer);

private slots:
  /** Indicate that it's time to check the status of the QAudioOutput buffer
   *  and send new data if it will fit. This is the 'heartbeat' of the class,
   *  and it is fired by the notify() signal of QAudioOutput. */
  void checkBuffer();

  /** Callback for the QAudioProbe when it received some data. It will open
   *  the output device if needed, and send along the buffer using the
   *  bufferReady() signal. */
  void handleBufferProbed(const QAudioBuffer& buffer);

private:
  /** Initialize the audio output device with the specified format.
   *  @param format the audio format we're playing back in.
   *  @param connect_notify if true, the notify() signal of QAudioOutput will
   *                        be connected to checkBuffer(). This is only needed
   *                        when we're decoding wav files directly. */
  void initAudioOutput(const QAudioFormat& format, bool connect_notify);

  /** Parse the header of a WAV file.
   *  @return bool if everything checks out, false if there was something wrong.
   */
  bool parseHeader();

  /** Search for a specified subchunk in m_file. If the subchunk is found, the
   *  file position is set to the start of the chunk. The file position should
   *  already be at the start of a subchunk and be before the subchunk to be
   *  found.
   *  @param identifier the four byte identifier of the subchunk
   *  @return true if the subchunk was found, false otherwise. */
  bool findSubChunk(const QString identifier);

  /** Read a word in little endian format from m_file and return it. This
   *  operation advances the m_file position by sizeof(word) bytes.
   *  This operation uses a little hack for reporting failure; when parsing
   *  headers, only positive values are expected but all variables are signed,
   *  so we report an error by returning -1.
   *  @return the length, or -1 on error. */
  template <typename word>
  word readNumber();

  QAudioOutput* m_audio_out        = NULL;
  QIODevice*    m_audio_out_device = NULL;

  QAudioProbe* m_probe;

  /** Indicite if we prefer to parse wav files ourselves. This will become true
   *  if QAudioProbe cannot connect to the QMediaPlayer (so we cannot intercept
   *  the raw audio data). */
  bool m_prefer_native_wav = false;

  /** Indicate if we're currently working with a wav file that we're playing
   *  natively. Otherwise QMediaPlayer is playing the current file. */
  bool m_is_native_wav = false;

  /** The format parameters of the audio file, if we parsed it natively. */
  QAudioFormat m_format;

  /** The wav file that we've opened. */
  QFile* m_file = NULL;

  /** The starting position in m_file of the raw audio data in a wav file, if we
   *  parsed it natively. */
  int m_data_offset = 0;

  /** The current time in the audio playback if we're playing a wav file
   *  natively. */
  qint64 m_time = 0;

  /** The duration of the loaded file if we parsed it natively. */
  qint64 m_duration = 0;

  /** Markers for the chunks and suchunks of WAV files. */
  const QString RIFF = "RIFF";
  const QString WAVE = "WAVE";
  const QString FMT  = "fmt ";
  const QString DATA = "data";
};

#endif // AUDIODECODER_H
