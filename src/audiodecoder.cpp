#include "audiodecoder.h"

AudioDecoder::AudioDecoder(QObject* parent) : QMediaPlayer(parent) {
#if defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
  m_prefer_native_wav = true;
#else
  // Try to set up an audio probe to intercept the raw audio data
  m_probe = new QAudioProbe(parent);
  if (m_probe->setSource(this)) {
    connect(m_probe, SIGNAL(audioBufferProbed(QAudioBuffer)),
            this,    SLOT(handleBufferProbed(QAudioBuffer)));
    setVolume(0);
  } else {
    // If we cannot set up a probe, we will play wav files natively.
    m_probe->deleteLater(); m_probe = NULL;
    m_prefer_native_wav = true;
  }
#endif
}

AudioDecoder::~AudioDecoder() {
  if (m_file)      m_file->deleteLater();
  if (m_audio_out) m_audio_out->deleteLater();
  if (m_probe)     m_probe->deleteLater();
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

QMediaPlayer::State AudioDecoder::state() const {
  if (m_is_native_wav) {
    return m_state_when_native;
  }
  return QMediaPlayer::state();
}

QMediaPlayer::MediaStatus AudioDecoder::mediaStatus() const {
  if (m_is_native_wav) {
    if (m_file->pos() == m_file->size()) {
      return EndOfMedia;
    }
    return LoadedMedia;  // If the m_is_native_wav flag is set, we have actually
                         // loaded a file.
  }
  return QMediaPlayer::mediaStatus();
}

bool AudioDecoder::isAudioAvailable() const {
  if (m_is_native_wav) return true;
  return QMediaPlayer::isAudioAvailable();
}

bool AudioDecoder::isIntercepting() {
  if (m_probe || m_is_native_wav) {
    return true;
  }
  return false;
}

QString AudioDecoder::getMediaPath() {
  if (m_is_native_wav) {
    if (m_file) {
      return QFileInfo(*m_file).absoluteFilePath();
    }
  } else {
    if (!media().isNull()) {
      QUrl url = media().canonicalUrl();
      if (url.isLocalFile()) {
        return url.toLocalFile();
      }
    }
  }

  return 0;
}

void AudioDecoder::setMedia(const QUrl& path) {
  pause();

  m_is_native_wav = false;

  // Reset the audio device
  if (m_audio_out) {
    m_audio_out->deleteLater();
    m_audio_out = NULL;
    m_audio_out_device = NULL;
  }

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
      m_file->deleteLater(); m_file = NULL;
    }

    // Open the file
    m_file = new QFile(path.toLocalFile());
    if (m_file->open(QIODevice::ReadOnly)) {
      if (parseHeader()) {
        m_is_native_wav = true;
        m_file->seek(m_data_offset);
        emit positionChanged(0);
        initAudioOutput(m_format, true);
        emit durationChanged(m_duration);
        emit mediaStatusChanged(LoadedMedia);
      }
    }
  }

  // Fall back on the QMediaPlayer if we couldn't or didn't want to open the
  // file natively.
  if (!m_is_native_wav) {
    QMediaPlayer::setMedia(path);
  }
}

void AudioDecoder::pause() {
  if (m_is_native_wav) {
    m_state_when_native = QMediaPlayer::PausedState;
    m_audio_out->suspend();
  }
  QMediaPlayer::pause();
}

void AudioDecoder::play() {
  if (m_is_native_wav) {
    if (m_audio_out != NULL) {
      m_state_when_native = QMediaPlayer::PlayingState;
      if (m_audio_out->state() == QAudio::SuspendedState) {
        // We're unpausing
        m_audio_out->resume();
      } else {
        // We start the playback by simply checking if we need to write data to
        // the buffer.
        checkBuffer();
      }
    }
  } else {
    QMediaPlayer::play();
  }
}

void AudioDecoder::setPosition(qint64 position) {
  if (m_is_native_wav) {
    if (position > m_duration) { // Cap
      position = m_duration;
    }

    // Set the position in the file to the desired location
    int file_pos = m_data_offset + m_format.bytesForDuration(position * 1000);
    m_file->seek(file_pos);

    m_time = position;
    emit positionChanged(position);
  }
  QMediaPlayer::setPosition(position);
}

void AudioDecoder::checkBuffer() {
  if (m_audio_out != NULL && m_state_when_native == QMediaPlayer::PlayingState) {

    while (m_audio_out->bytesFree() >= m_audio_out->periodSize()) {
      // We can append data to the buffer, so send some new data
      QByteArray data = m_file->read(m_audio_out->periodSize());
      if (data.length() > 0) {
        QAudioBuffer buffer(data, m_format, m_time);
        m_time += (m_format.durationForBytes(data.length()) / 1000);
        emit bufferReady(buffer);
        emit positionChanged(m_time); // TODO: Fire less often
      }
      if (data.length() < m_audio_out->periodSize()) {
        emit mediaStatusChanged(EndOfMedia);
        m_state_when_native = QMediaPlayer::StoppedState;
        break;
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
  if (m_audio_out) {
    m_audio_out->deleteLater();
  }
  m_audio_out = new QAudioOutput(format);
  m_audio_out_device = m_audio_out->start();

  if (connect_notify) {
    // We need to check and fill the buffer a bit faster than we send data to it
    // to make sure it is kept full. Therefore, we connect to the notify() signal
    // and set the interval time to half that of the amount of time we sent with
    // each buffer.
    // We specifically ask for a QueuedConnection because we don't want checkBuffer()
    // to be called when it is still running.
    connect(m_audio_out, SIGNAL(notify()),
            this,        SLOT(checkBuffer()), Qt::QueuedConnection);
    m_audio_out->setNotifyInterval(
            m_format.durationForBytes(m_audio_out->periodSize()) / 20000); // us->ms
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
  } else {
    // Not an actual wave file
    return false;
  }

  // Sanity check for the lenght of the file
  qint32 remainder = readNumber<qint32>();
  if (m_file->size() != remainder + 8) {
    return false;
  }

  // Last part of the signature
  bytes = m_file->read(4);
  if (bytes.length() != 4) return false;
  if (bytes != WAVE) return false;

  if (findSubChunk(FMT)) {
    m_file->seek(m_file->pos() + 4);

    if (readNumber<qint32>() != 16) {
      // No PCM data
      return false;
    } else {
      m_format.setCodec("audio/pcm");
    }
    if (readNumber<qint16>() != 1) return false; // Compressed

    qint16 num_channels = readNumber<qint16>();
    if (num_channels == -1) {
      return false;
    } else {
      m_format.setChannelCount(num_channels);
    }

    qint32 sample_rate = readNumber<qint32>();
    if (sample_rate == -1) {
      return false;
    } else {
      m_format.setSampleRate(sample_rate);
    }

    m_file->seek(m_file->pos() + 6); // These bytes aren't that interesting,
                                     // they are products of the other
                                     // parameters.
    qint16 bits_per_sample = readNumber<qint16>();
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
      qint64 num_bytes = (qint64)readNumber<qint32>();
      m_duration = (num_bytes * 1000) / (num_channels * sample_rate * (bits_per_sample / 8));
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
    qint32 remainder = readNumber<qint32>();
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
word AudioDecoder::readNumber() {
  QByteArray bytes = m_file->read(sizeof(word));
  if (bytes.length() != sizeof(word)) {
    return -1;
  }

  const word* interpreted = reinterpret_cast<const word*>(bytes.constData());
  return qFromLittleEndian(interpreted[0]);
}

