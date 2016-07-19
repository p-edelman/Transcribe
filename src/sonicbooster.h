#ifndef SONICBOOSTER_H
#define SONICBOOSTER_H

#include <QObject>

#include <QAudioBuffer>
#include <QDebug>
#include <QtGlobal>
#include <QtMath>

#include <cstdint>
#include <limits>

/** Boost an audio buffer, so that it plays more loudly. This class is a 'slave'
 *  class to an audio player; it should constantly be fed short QAudioBuffers as
 *  input. The boosted audio is stored in an internal raw buffer, which can be
 *  requested with getBoostedBuffer() and getBoostedBufferSize().
 *  The boost amount can be set adjusted by the user. This number is not always
 *  used however; if the amount of clipping would become to large for a given
 *  audio buffer, it is scaled down. Thus loud parts of the audio stream will
 *  not be clipped too much.
 *  NOTE: Only 8 and 16 bit signed or unsigned data can be boosted. You can
 *  check if a buffer can be boosted with the canBoost() method.
 */
class SonicBooster : public QObject {
  Q_OBJECT

public:
  SonicBooster(QObject* parent = 0);
  ~SonicBooster();

  /** Return the user specified boost factor.
   *  Note that the actual boost factor may be adjusted to a lower value if the
   *  amount of clipping in a buffer would get too high. */
  qreal getFactor();

  /** Indicate if the signal with the given audio format can be amplified. */
  bool canBoost(const QAudioFormat& format);

  /** Boost the given audio buffer. If succesful, the boosted audio data is
   *  available through the getBoostedBuffer() method. The return value should
   *  always be used to check if this buffer is available!
   *  @param buffer the QAudioBuffer that should be boosted.
   *  @return true if the audio is boosted, false otherwise. If the return value
   *               is false, getBoostedBuffer() doesn't contain valid data! */
  bool boost(const QAudioBuffer& buffer);

  /** Return the raw boosted audio data. This can be used to write to a
   *  QIODevice opened by QAudioOutput.
   *  @param size will hold the number of bytes in the buffer. This will be 0
   *              if the last boost() operation failed.
   *  @return a pointer to the raw audio data. */
  const char* getBoostedBuffer(int& size);

public slots:
  /** Increase of decrease the boost factor by 0.1 */
  void increaseFactor();
  void decreaseFactor();

private:
  /** Set the boost factor. Negative values will be clipped at 0. */
  void setFactor(qreal factor);

  /** Amplify the audio by the m_boost factor and store the result in the
   *  supplied data buffer. Check the return value to make sure the data in
   *  buffer is valid before using!
   *  If the signal is boosted outside its bounds, it will be clipped.
   *  @tparam word_type the type of the audio data.
   *  @param data the raw data
   *  @param num_samples the number of samples in data (not the number of bytes)
   *  @return true if the signal was modified, false otherwise. */
  template<typename word_type> bool boostAudioBuffer(const word_type* data,
                                                     int num_samples);

  /** Calculate the max boost factor that can be applied to the buffer without
   *  too much clipping (where 'too much' is defined at ten per cent of the
   *  samples. */
  template<typename word_type> qreal getMaxFactor(const word_type* data,
                                                  int num_samples);

  /** Rewrite unsigned data in in_buffer to signed data in out_buffer, or vice
   *  versa.
   *  @tparam from_type the type to convert from
   *  @tparam to_type the type to convert to. It should have a different
   *                  signedness than from_type, and the same bit width.
   *  @param in_buffer the buffer with the input data
   *  @param out_buffer the buffer for the modified data. It should be large
   *                    enough to accomodate all the data
   *  @param num_samples the number of elements in both buffers.
   */
  template<typename from_type, typename to_type>
  void switchSignedness(const from_type* in_buffer,
                        to_type* out_buffer,
                        int num_samples);

  /** Enlarge the size of m_data if the supplied buffer wouldn't fit. We never
   *  decrease it. */
  void adjustDataBufferSize(const QAudioBuffer& buffer);

  /** The factor that audio data is multiplied with before playing. */
  qreal m_factor = 1.0;

  /** Due to restrictions with the QMediaPlayer/QAudioProbe setup we can only
   *  get const audio data. The result of all operations is thus copied to
   *  another buffer. We declared it once to reduce the overhead of repeated
   *  memory operations. This buffer will also hold the final and is returned
   *  by getBoostedBuffer(). */
  char* m_data = NULL;

  /** Remember the size of the audio data buffer, so that we can enlarge it if a
   *  new buffer arrives which requires more space. We never reduce the size
   *  though, as audio buffers are fairly constant in size and it wouldn't make
   *  much sense to reclaim the small amount of memory. */
  int m_data_max_bytes = 0;

  /** Keep track of the number of bytes in m_data after the last boost()
   *  operation. If the boost() operation didn't succeed, this number will b
   *  zero. */
  int m_boosted_data_bytes = 0;

  /** To figure out by how much we can boost an audio sample without clipping it
   *  too much, we have to count how many of each sample point there are.
   *  We use int for this as the QAudioFormat::sampleCount() method uses this as
   *  the return type, thus the actual count fits into an int. */
  QVector<int>* m_spectrogram = NULL;
};

#endif // SONICBOOSTER_H
