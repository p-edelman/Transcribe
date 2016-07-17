#include "sonicbooster.h"

SonicBooster::SonicBooster(QObject *parent) : QObject(parent) {
  m_spectrogram = new QVector<int>;
}

SonicBooster::~SonicBooster() {
  delete m_modified_buffer;
}

bool SonicBooster::boost(const QAudioBuffer& buffer) {
  m_is_last_modified = false;

  if (buffer.isValid()) {
    adjustModifiedBufferSize(buffer);

    if (buffer.format().sampleType() == QAudioFormat::SignedInt) {
      if (buffer.format().sampleSize() == 8) {
        m_is_last_modified = boostAudioBuffer<int8_t>(buffer);
      } else if (buffer.format().sampleSize() == 16) {
        m_is_last_modified = boostAudioBuffer<int16_t>(buffer);
      } else {
        qDebug() << "Unsupported bit depth: " << buffer.format().sampleSize();
      }
    } else {
      // TODO: Unreadable
      qDebug() << "Unsupported format";
    }
  }

  return m_is_last_modified;
}

const char* SonicBooster::getBoostedBuffer(int& size) {
  if (m_is_last_modified) {
    size = m_modified_buffer_size;
  } else {
    size = 0;
  }
  return m_modified_buffer;
}

qreal SonicBooster::getFactor() {
  return m_factor;
}

void SonicBooster::increaseFactor() {
  setFactor(m_factor + 0.1);
}

void SonicBooster::decreaseFactor() {
  setFactor(m_factor - 0.1);
}

void SonicBooster::setFactor(qreal factor) {
  if (factor > 0) {
    m_factor = factor;
  }
  qDebug() << "Boost factor is now " << m_factor;
}

template<class word_type>
bool SonicBooster::boostAudioBuffer(const QAudioBuffer& buffer) {
  if (m_factor != 1.0) {
    // If we amplify the audio signal, we need to check if the amount of clipped
    // samples is acceptible (smaller than ten percent).
    qreal factor = m_factor;
    if (m_factor> 1.0) {
      factor = getMaxFactor<word_type>(buffer);
      if (factor != m_factor) {
        qDebug() << "Using actual factor of " << factor;
      }
    }

    const word_type* data = buffer.constData<word_type>();
    word_type* new_buffer = (word_type*)m_modified_buffer;
    for (int i = 0; i < buffer.sampleCount(); i++) {
      // qint32 should be sufficient as we only handle 8 and 16 bit data.
      qint32 val = static_cast<qint32>(data[i]) * factor;

      // Cap the value if needed
      if (val > std::numeric_limits<word_type>::max()) {
        new_buffer[i] = std::numeric_limits<word_type>::max();
      } else if (val < std::numeric_limits<word_type>::min()) {
        new_buffer[i] = std::numeric_limits<word_type>::min();
      } else {
        new_buffer[i] = static_cast<word_type>(val);
      }
    }
    return true;
  }

  return false;
}

template<class word_type>
qreal SonicBooster::getMaxFactor(const QAudioBuffer& buffer) {
  // Resize the spectrogram size if needed.
  if (abs(std::numeric_limits<word_type>::min()) > m_spectrogram->size()) {
    m_spectrogram->resize(abs(std::numeric_limits<word_type>::min()));
  }
  if (std::numeric_limits<word_type>::max() > m_spectrogram->size()) {
    m_spectrogram->resize(std::numeric_limits<word_type>::max());
  }

  // Calculate the value at which samples will be clipped
  qreal factor = m_factor;
  word_type cutoff = std::numeric_limits<word_type>::max() / factor;

  // Create a reverse cumulative count of the number of samples for the
  // relevant part of the spectrum (everything between the cutoff and the
  // max allowable value.
  for (int i = cutoff; i < std::numeric_limits<word_type>::max(); i++) {
    m_spectrogram->data()[i] = 0;
  }
  const word_type* data = buffer.constData<word_type>();
  for (int i = 0; i < buffer.sampleCount(); i++) {
    m_spectrogram->data()[abs(data[i])]++;
  }
  for (int i = std::numeric_limits<word_type>::max() - 1; i > cutoff - 1; i--) {
    m_spectrogram->data()[i] += m_spectrogram->data()[i + 1];
  }

  // Now we can stepwise scale down the boost factor until the number of
  // clipped samples falls below ten percent.
  int fraction = buffer.sampleCount() / 10;
  while (m_spectrogram->data()[cutoff] >= fraction) {
    factor -= 0.1;
    cutoff = std::numeric_limits<word_type>::max() / factor;
  }

  return factor;
}

void SonicBooster::adjustModifiedBufferSize(const QAudioBuffer& buffer) {
  if (buffer.byteCount() > m_modified_buffer_size) {
    qDebug() << "Resizing buffer";
    m_modified_buffer_size = buffer.byteCount();
    m_modified_buffer      = (char*)realloc(m_modified_buffer, m_modified_buffer_size);
  }
}
