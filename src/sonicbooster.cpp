#include "sonicbooster.h"

SonicBooster::SonicBooster(QObject* parent) : QObject(parent) {}

SonicBooster::~SonicBooster() {
  delete m_data;
  delete m_spectrogram;
}

bool SonicBooster::canBoost(const QAudioFormat& format) {
  switch (format.sampleType()) {
    case QAudioFormat::SignedInt:
    case QAudioFormat::UnSignedInt:
      break;
    default:
      return false;
  }

  switch (format.sampleSize()) {
    case 8:
    case 16:
      break;
    default:
      return false;
  }

  return true;
}

int SonicBooster::level() {
  return round(m_level);
}

bool SonicBooster::boost(const QAudioBuffer& buffer) {
  if (!(canBoost(buffer.format()) && buffer.isValid())) {
    return false;
  }

  adjustDataBufferSize(buffer);

  char* data = (char*)buffer.constData();
  int   size = buffer.sampleCount();

  // If format is unsigned, make a signed copy of the data and use that for the
  // actual amplification.
  if (buffer.format().sampleType() == QAudioFormat::UnSignedInt) {
    if (buffer.format().sampleSize() == 8) {
      switchSignedness((uint8_t*)data, (int8_t*)m_data, size);
    } else if (buffer.format().sampleSize() == 16) {
      switchSignedness((uint16_t*)data, (int16_t*)m_data, size);
    }
    data = m_data;
  }

  // Boost the signal
  bool is_modified = false;
  if (buffer.format().sampleSize() == 8) {
    is_modified = boostAudioBuffer((int8_t*)data, size);
  } else if (buffer.format().sampleSize() == 16) {
    is_modified = boostAudioBuffer((int16_t*)data, size);
  }

  // If needed, convert the modified buffer back to unsigned
  if (buffer.format().sampleType() == QAudioFormat::UnSignedInt) {
    if (buffer.format().sampleSize() == 8) {
      switchSignedness((int8_t*)m_data, (uint8_t*)m_data, size);
    } else if (buffer.format().sampleSize() == 16) {
      switchSignedness((int16_t*)m_data, (uint16_t*)m_data, size);
    }
  }

  return is_modified;
}

const char* SonicBooster::getBoostedBuffer(int& size) {
  size = m_boosted_data_bytes;
  return m_data;
}

template<typename word_type>
bool SonicBooster::boostAudioBuffer(const word_type* data, int num_samples) {

  m_boosted_data_bytes = 0;

  // Calculate the amplification factor for the samples
  qreal factor = qPow(10, m_level / 20);

  if (factor != 1.0) {
    // If we amplify the audio signal, we need to check if the amount of clipped
    // samples is acceptible (smaller than ten percent).
    if (factor > 1.0) {
      factor = getMaxFactor<word_type>(factor, data, num_samples);
    }

    word_type* new_buffer = (word_type*)m_data;
    for (int i = 0; i < num_samples; i++) {
      // qint32 should be sufficient as we only handle 8 and 16 bit data.
      qint32 val = static_cast<qint32>(data[i]) * factor;

      // Cap the value if needed
      if (val > std::numeric_limits<word_type>::max()) {
        val = std::numeric_limits<word_type>::max();
      } else if (val < std::numeric_limits<word_type>::min()) {
        val = std::numeric_limits<word_type>::min();
      }
      new_buffer[i] = static_cast<word_type>(val);
    }
    m_boosted_data_bytes = num_samples * sizeof(word_type);
    return true;
  }

  return false;
}

template<typename word_type>
qreal SonicBooster::getMaxFactor(qreal factor,
                                 const word_type* data, int num_samples) {
  // Resize the spectrogram size if needed.
  if (abs(std::numeric_limits<word_type>::min()) > m_spectrogram->size()) {
    m_spectrogram->resize(abs(std::numeric_limits<word_type>::min()));
  }
  if (std::numeric_limits<word_type>::max() > m_spectrogram->size()) {
    m_spectrogram->resize(std::numeric_limits<word_type>::max());
  }

  // Calculate the value at which samples will be clipped
  word_type cutoff = std::numeric_limits<word_type>::max() / factor;

  // Create a reverse cumulative count of the number of samples for the
  // relevant part of the spectrum (everything between the cutoff and the
  // max allowable value).
  for (int i = cutoff; i < std::numeric_limits<word_type>::max(); i++) {
    m_spectrogram->data()[i] = 0;
  }
  for (int i = 0; i < num_samples; i++) {
    m_spectrogram->data()[(word_type)abs(data[i])]++;
  }
  for (int i = std::numeric_limits<word_type>::max() - 1; i > cutoff - 1; i--) {
    m_spectrogram->data()[i] += m_spectrogram->data()[i + 1];
  }

  // Now we can stepwise scale down the boost factor until the number of
  // clipped samples falls below ten percent.
  int fraction = num_samples / 10;
  while (m_spectrogram->data()[cutoff] > fraction) {
    factor -= 0.1;
    cutoff = std::numeric_limits<word_type>::max() / factor;
  }

  return factor;
}

template<typename from_type, typename to_type>
void SonicBooster::switchSignedness(const from_type* in_buffer,
                                    to_type* out_buffer,
                                    int num_samples) {
  qint32 sign_offset = (1 << (sizeof(to_type) * 8 - 1));
  bool to_signed = std::numeric_limits<to_type>::is_signed;
  for (int i = 0; i < num_samples; i++) {
    if (to_signed) {
      out_buffer[i] = in_buffer[i] - sign_offset;
    } else {
      out_buffer[i] = in_buffer[i] + sign_offset;
    }
  }
}

void SonicBooster::adjustDataBufferSize(const QAudioBuffer& buffer) {
  if (buffer.byteCount() > m_data_max_bytes) {
    qDebug() << "Resizing buffer";
    m_data_max_bytes = buffer.byteCount();
    m_data           = (char*)realloc(m_data,
                                      m_data_max_bytes);
  }
}
