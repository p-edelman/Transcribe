#include <QtTest>

#include "audioplayer.h"

class AudioPlayerTest : public QObject {
  Q_OBJECT

public:
  AudioPlayerTest();

private Q_SLOTS:
  void initAudioPlayer();
};

AudioPlayerTest::AudioPlayerTest() {}

/** Test the initialization of the AudioPlayer. */
void AudioPlayerTest::initAudioPlayer() {
  AudioPlayer player;
  QVERIFY(player.getPlayerState() == AudioPlayer::PAUSED);
  QVERIFY(player.getDuration() == 0);
  QVERIFY(player.getPosition() == 0);
}

QTEST_APPLESS_MAIN(AudioPlayerTest)

#include "tst_audioplayertest.moc"
