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
 *  used however; if the amount of clipping would become to great for a given
 *  audio buffer, it is scaled down. So loud parts of the audio stream will
 *  not be clipped too much.
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

  /** Boost the given audio buffer. If succesful, the boosted audio data is
   *  available through the getBoostedBuffer() method. The return value should
   *  always be used to check if this buffer is available!
   *  @param buffer the QAudioBuffer that should be boosted.
   *  @return true if the audio is boosted, false otherwise. If the return value
   *               is false, getBoostedBuffer() doesn't contain valid data!
   */
  bool boost(const QAudioBuffer& buffer);

  /** Return the raw boosted audio data. This can primarily be used to write to
   *  a QIODevice opened by QAudioOutput.
   *  @param size will hold the number of bytes in the buffer.
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
   *  If the signal is boosted outside its bound, it will be clipped.
   *  @tparam word_type the type of the audio data.
   *  @param buffer the audio buffer.
   *  @return true if the signal was modified, false otherwise. If false is
                   returned, the data in m_modified_buffer is invalid! */
  template<class word_type> bool boostAudioBuffer(const QAudioBuffer& buffer);

  /** Calculate the max boost factor that can be applied to the buffer without
   *  too much clipping (where 'too much' is defined at ten per cent of the
   *  samples. */
  template<class word_type> qreal getMaxFactor(const QAudioBuffer& buffer);

  /** Enlarge the size of m_modified_buffer if the supplied buffer wouldn't
   *  fit. We never decrease it. */
  void adjustModifiedBufferSize(const QAudioBuffer& buffer);

  /** The factor that audio data is multiplied with before playing. */
  qreal m_factor = 1.0;

  /** Buffer for storing modified audio data. We declare it just once to avoid
   *  the overhead of repeated memory operations. */
  char* m_modified_buffer = NULL;

  /** Remember the size of the modified audio data buffer, so that we can
   *  enlarge it if a new buffer arrives which requires more space. We never
   *  reduce the size though, as audio buffers are fairly constant in size and
   *  it wouldn't make much sense to reclaim the small amount of memory. */
  int m_modified_buffer_size = 0;

  /** Keep track of whether the last boost() operation succeeded. */
  bool m_is_last_modified;

  /** To figure out by how much we can boost an audio sample without clipping it
   *  too much, we have to count how many of each sample point there are.
   *  We use int for this as the QAudioFormat::sampleCount() method uses this as
   *  the return type, thus the actual count fits into an int. */
  QVector<int>* m_spectrogram = NULL;
};

#endif // SONICBOOSTER_H
