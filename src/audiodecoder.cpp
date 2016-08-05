#include "audiodecoder.h"

AudioDecoder::AudioDecoder(QObject* parent) : QMediaPlayer(parent) {
  // Try to set up an audio probe to intercept the raw audio data
  m_probe = new QAudioProbe(parent);
  if (m_probe->setSource(this)) {
    connect(m_probe, SIGNAL(audioBufferProbed(QAudioBuffer)),
            this,    SLOT(handleBufferProbed(QAudioBuffer)));
    setVolume(0);
  } else {
    // If we cannot set up a probe, we will play wav files natively.
    delete m_probe; m_probe = NULL;
    m_prefer_native_wav = true;
  }
}

AudioDecoder::~AudioDecoder() {
  delete m_file;
  delete m_audio_out;
  delete m_probe;
}

qint64 AudioDecoder::duration() const {
  if (m_is_native_wav) {
    return m_duration;
  }
  return QMediaPlayer::duration();
}

qint64 AudioDecoder::position() const {
  if (m_is_native_wav) {
    return m_time;
  }
  return QMediaPlayer::position();
}

QMediaPlayer::MediaStatus AudioDecoder::mediaStatus() const {
  if (m_is_native_wav) { // This flag is only set if we actually loaded media
    return QMediaPlayer::LoadedMedia;
  }
  return QMediaPlayer::mediaStatus();
}

void AudioDecoder::setMedia(const QUrl& path) {
  m_is_native_wav = false;

  // Reset the audio device
  delete m_audio_out;
  m_audio_out = NULL;
  m_audio_out_device = NULL;

  // Try to load native wav
  if (m_prefer_native_wav) {
    // Reset all persistent data
    m_time        = 0;
    m_duration    = 0;
    m_data_offset = 0;
    if (m_file) {
      if (m_file->isOpen()) {
        m_file->close();
      }
      delete m_file; m_file = NULL;
    }

    // Open the file
    m_file = new QFile(path.toLocalFile());
    if (m_file->open(QIODevice::ReadOnly)) {
      if (parseHeader()) {
        m_is_native_wav = true;
        m_file->seek(m_data_offset);
        initAudioOutput(m_format, true);
        emit durationChanged(m_duration);
        emit mediaStatusChanged(QMediaPlayer::LoadedMedia);
        qDebug() << "Media loaded";
      } else {
        qDebug() << "Header not parsed";
        // TODO: emit an error
      }
    } else {
      qDebug() << "Couldn't open file";
      // TODO: emit an error
    }
  }

  // Fall back on the QMediaPlayer if we couldn't or didn't want to open the
  // file natively.
  if (!m_is_native_wav) {
    QMediaPlayer::setMedia(path);
  }
}

void AudioDecoder::play() {
  if (m_is_native_wav) {
    if (m_audio_out != NULL) {
      // We simply start the playback by checking if we need to write data to
      // the buffer.
      checkBuffer();
    }
  } else {
    QMediaPlayer::play();
  }
}

void AudioDecoder::checkBuffer() {
  if (m_audio_out != NULL) {
    if (m_audio_out->bytesFree() >= m_audio_out->periodSize()) {
      // We can append data to the buffer again, so send some new data
      QByteArray data = m_file->read(m_audio_out->periodSize());
      if (data.length() > 0) {
        QAudioBuffer buffer(data, m_format, m_time);
        m_time += (m_format.durationForBytes(data.length()) / 1000);
        emit bufferReady(buffer);
        emit positionChanged(m_time); // TODO: Fire less often
      } else {
        emit mediaStatusChanged(QMediaPlayer::EndOfMedia);
      }
    }
  }
}

void AudioDecoder::handleBufferProbed(const QAudioBuffer& buffer) {
  // There is no other way to get the audio format using QAudioProbe than to
  // wait for a buffer. The first time we get it, we can open the QAudioDevice.
  if (m_audio_out == NULL) {
    initAudioOutput(buffer.format(), false);
  }
  emit bufferReady(buffer);
}

void AudioDecoder::initAudioOutput(const QAudioFormat& format,
                                bool connect_notify) {
  delete m_audio_out;

  m_audio_out = new QAudioOutput(format);
  m_audio_out_device = m_audio_out->start();

  if (connect_notify) {
    // We need to check and fill the buffer a bit faster than we send data to it
    // to make sure it is kept full. Therefore, we connect to the notify() signal
    // and set the interval time to half that of the amount of time we sent with
    // each buffer.
    connect(m_audio_out, SIGNAL(notify()),
            this,        SLOT(checkBuffer()));
    m_audio_out->setNotifyInterval(
          m_format.durationForBytes(m_audio_out->periodSize()) / 2000); // us->ms
  }
}

bool AudioDecoder::parseHeader() {
  m_file->seek(0);
  m_data_offset = 0;

  QByteArray bytes;

  bytes = m_file->read(4);
  if (bytes.length() != 4) return false;
  if (bytes == RIFF) { // WAV file
    m_format.setByteOrder(QAudioFormat::LittleEndian);
  } else if (bytes == FORM) { // AIFF file
    m_format.setByteOrder(QAudioFormat::BigEndian);
  } else {
    // Not an actual wave file
    return false;
  }

  // Sanity check for the lenght of the file
  qint32 remainder = readNumber<qint32>(m_format.byteOrder());
  if (m_file->size() != remainder + 8) {
    return false;
  }

  // Last part of the signature
  bytes = m_file->read(4);
  if (bytes.length() != 4) return false;
  if ((m_format.byteOrder() == QAudioFormat::LittleEndian && bytes != WAVE) ||
      (m_format.byteOrder() == QAudioFormat::BigEndian    && bytes != AIFF)) {
    return false;
  }

  if (m_format.byteOrder() == QAudioFormat::LittleEndian) {
    return extractWAVEParams();
  } else {
    return extractAIFFParams();
  }

  return false;
}

bool AudioDecoder::extractWAVEParams() {
  if (findSubChunk(FMT)) {
    m_file->seek(m_file->pos() + 4);

    if (readNumber<qint32>(QAudioFormat::LittleEndian) != 16) {
      // No PCM data
      return false;
    } else {
      m_format.setCodec("audio/pcm");
    }
    if (readNumber<qint16>(QAudioFormat::LittleEndian) != 1) {
      // Compressed
      return false;
    }

    qint16 num_channels = readNumber<qint16>(QAudioFormat::LittleEndian);
    if (num_channels == -1) {
      return false;
    } else {
      m_format.setChannelCount(num_channels);
    }

    qint32 sample_rate = readNumber<qint32>(QAudioFormat::LittleEndian);
    if (sample_rate == -1) {
      return false;
    } else {
      m_format.setSampleRate(sample_rate);
    }

    m_file->seek(m_file->pos() + 6); // These bytes aren't that interesting,
                                     // they are products of the other
                                     // parameters.
    qint16 bits_per_sample = readNumber<qint16>(QAudioFormat::LittleEndian);
    if (bits_per_sample == -1) {
      return false;
    } else {
      m_format.setSampleSize(bits_per_sample);
      if (bits_per_sample == 8) {
        m_format.setSampleType(QAudioFormat::UnSignedInt);
      } else if (bits_per_sample == 16) {
        m_format.setSampleType(QAudioFormat::SignedInt);
      } else {
        return false;
      }
    }

    // Now find the data chunk and set the file position to it
    if (findSubChunk(DATA)) {
      m_data_offset = m_file->pos() + 8;

      // Calculate the length of the file
      m_file->seek(m_data_offset - 4);
      qint32 num_bytes = readNumber<qint32>(QAudioFormat::LittleEndian);
      m_duration = (num_bytes * 1000) / (num_channels * sample_rate * (bits_per_sample / 8));
      return true;
    }
  }
  return false;
}

bool AudioDecoder::extractAIFFParams() {
  if (findSubChunk(COMM)) {
    m_file->seek(m_file->pos() + 4);

    // Always the same for AIFF
    m_format.setCodec("audio/pcm");
    m_format.setSampleType(QAudioFormat::SignedInt);

    qint32 remainder = readNumber<qint32>(QAudioFormat::BigEndian);

    qint16 num_channels = readNumber<qint16>(QAudioFormat::BigEndian);
    if (num_channels == -1) {
      return false;
    } else {
      m_format.setChannelCount(num_channels);
    }

    m_file->seek(m_file->pos() + 4); // Number of sample frames

    qint16 bits_per_sample = readNumber<qint16>(QAudioFormat::BigEndian);
    if (bits_per_sample == 8 || bits_per_sample == 16) {
      m_format.setSampleSize(bits_per_sample);
    } else {
      return false;
    }

    // The fraction is somehow stored as a 10 byte extended real. There doesn't
    // seem a reliable cross-platform way to access this data type however, so
    // we have to parse it manually.
    QByteArray bytes = m_file->read(10);
    if (bytes.length() != 10) return false;
    if (((bytes[0] >> 7) & (1 << 7)) != 0) return false; // Negative number;
    // Bit 1..15 contains the exponent
    qint16 exponent = qFromBigEndian(reinterpret_cast<qint16*>(bytes.data())[0]);
    exponent = exponent & ~(1 << 15); // Shift of first bit
    exponent -= 16383;                // Subtract bias
    if ((bytes[2] & (1 << 7)) == 0) return false; // Non-normalized number, we can't handle it;
    // Bit 17..79 contain the fraction (the bits after the comma)
    quint64 fraction = qFromBigEndian(reinterpret_cast<quint64*>(bytes.data() + 2)[0]);
    fraction = ((fraction << 1) >> 1); // Shift of the first bit
    // We can now calculate the actual value with (1.fraction * 2^exponent)
    // This is the same as 2^exponent + fraction * 2^-63 * 2^exponent, or
    // 2^exponent + fraction * 2^exponent-63
    qreal rate = (2 << (exponent - 1)) + fraction * pow(2, exponent - 63);
    m_format.setSampleRate(round(rate));

    // In principle the subchunk should be finished now, but it is possible that
    // it is longer so we should skip to the end of it.
    m_file->seek(m_file->pos() + (remainder - 18));

    // Now find the data chunk and set the file position to it
    if (findSubChunk(SSND)) {
      m_data_offset = m_file->pos() + 6;
      return true;
    }
  }
  return false;
}

bool AudioDecoder::findSubChunk(const QString identifier) {
  // Read the identifier of the current chunk
  QByteArray bytes = m_file->read(4);
  if (bytes.length() != 4) return false;

  while (bytes != identifier) {
    // Seek to the next subchunk and read its signature, or return false if
    // we're at the end
    qint32 remainder = readNumber<qint32>(m_format.byteOrder());
    if (remainder == -1) return false;
    m_file->seek(m_file->pos() + remainder);
    bytes = m_file->read(4);
    if (bytes.length() != 4) return false;
  }

  // If the identifier matches, rewind 4 bytes and report success
  m_file->seek(m_file->pos() - 4);
  return true;
}

template <typename word>
word AudioDecoder::readNumber(QAudioFormat::Endian endianness) {
  QByteArray bytes = m_file->read(sizeof(word));
  if (bytes.length() != sizeof(word)) {
    return -1;
  }

  const word* interpreted = reinterpret_cast<const word*>(bytes.constData());
  if (endianness == QAudioFormat::BigEndian) {
    return qFromBigEndian(interpreted[0]);
  } else {
    return qFromLittleEndian(interpreted[0]);
  }
}

