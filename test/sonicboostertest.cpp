#include "sonicboostertest.h"

SonicBoosterTest::SonicBoosterTest() {
  m_booster_m_6 = new SonicBooster();
  m_booster_p_6 = new SonicBooster();
  m_boost_factor_m_6 = qPow(10.0, -6.0 / 20.0);
  m_boost_factor_p_6 = qPow(10.0, 6.0 / 20.0);

  for (int i=0; i < 6; i++) {
    m_booster_m_6->decreaseLevel();
    m_booster_p_6->increaseLevel();
  }
}

template<class word_type> QAudioBuffer SonicBoosterTest::getBuffer() {
  QAudioFormat format;
  format.setChannelCount(1);
  format.setCodec("audio/pcm");
  format.setSampleRate(44100);
  format.setSampleSize(sizeof(word_type) * 8);
  if (std::numeric_limits<word_type>::is_signed) {
    format.setSampleType(QAudioFormat::SignedInt);
  } else {
    format.setSampleType(QAudioFormat::UnSignedInt);
  }

  word_type raw_data[3];
  QByteArray data((char*)raw_data, 3 * sizeof(word_type));
  QAudioBuffer buffer(data, format, -1);

  return buffer;
}

void SonicBoosterTest::signed8Data() {
  QAudioBuffer buffer = getBuffer<qint8>();

  qint8* raw_data = (qint8*)buffer.data();
  raw_data[0] = 0;
  raw_data[1] = 50;
  raw_data[2] = -50;

  QVERIFY(m_booster_m_6->boost(buffer));
  QVERIFY(m_booster_p_6->boost(buffer));

  int len0_5 = 0;
  const qint8* boosted0_5 = (qint8*)m_booster_m_6->getBoostedBuffer(len0_5);
  QCOMPARE(len0_5, 3);
  QCOMPARE((int)boosted0_5[0], 0);
  QCOMPARE((int)boosted0_5[1], (int)(50 * m_boost_factor_m_6));
  QCOMPARE((int)boosted0_5[2], (int)(-50 * m_boost_factor_m_6));

  int len1_5 = 0;
  const qint8* boosted1_5 = (qint8*)m_booster_p_6->getBoostedBuffer(len1_5);
  QCOMPARE(len1_5, 3);
  QCOMPARE((int)boosted1_5[0], 0);
  QCOMPARE((int)boosted1_5[1], (int)(50 * m_boost_factor_p_6));
  QCOMPARE((int)boosted1_5[2], (int)(-50 * m_boost_factor_p_6));
}

void SonicBoosterTest::signed16Data() {
  QAudioBuffer buffer = getBuffer<qint16>();

  qint16* raw_data = (qint16*)buffer.data();
  raw_data[0] = 0;
  raw_data[1] = 200;
  raw_data[2] = -200;

  QVERIFY(m_booster_m_6->boost(buffer));
  QVERIFY(m_booster_p_6->boost(buffer));

  int len0_5 = 0;
  const qint16* boosted0_5 = (qint16*)m_booster_m_6->getBoostedBuffer(len0_5);
  QCOMPARE(len0_5, 6);
  QCOMPARE((int)boosted0_5[0], 0);
  QCOMPARE((int)boosted0_5[1], (int)(200 * m_boost_factor_m_6));
  QCOMPARE((int)boosted0_5[2], (int)(-200 * m_boost_factor_m_6));

  int len1_5 = 0;
  const qint16* boosted1_5 = (qint16*)m_booster_p_6->getBoostedBuffer(len1_5);
  QCOMPARE(len1_5, 6);
  QCOMPARE((int)boosted1_5[0], 0);
  QCOMPARE((int)boosted1_5[1], (int)(200 * m_boost_factor_p_6));
  QCOMPARE((int)boosted1_5[2], (int)(-200 * m_boost_factor_p_6));
}

void SonicBoosterTest::unsigned8Data() {
  QAudioBuffer buffer = getBuffer<quint8>();

  quint8* raw_data = (quint8*)buffer.data();
  raw_data[0] = 128;
  raw_data[1] = 128 + 50;
  raw_data[2] = 128 - 50;

  QVERIFY(m_booster_m_6->boost(buffer));
  QVERIFY(m_booster_p_6->boost(buffer));

  int len0_5 = 0;
  const quint8* boosted0_5 = (quint8*)m_booster_m_6->getBoostedBuffer(len0_5);
  QCOMPARE(len0_5, 3);
  QCOMPARE((int)boosted0_5[0], 128);
  QCOMPARE((int)boosted0_5[1], 128 + (int)(50 * m_boost_factor_m_6));
  QCOMPARE((int)boosted0_5[2], 128 - (int)(50 * m_boost_factor_m_6));

  int len1_5 = 0;
  const quint8* boosted1_5 = (quint8*)m_booster_p_6->getBoostedBuffer(len1_5);
  QCOMPARE(len1_5, 3);
  QCOMPARE((int)boosted1_5[0], 128);
  QCOMPARE((int)boosted1_5[1], 128 + (int)(50 * m_boost_factor_p_6));
  QCOMPARE((int)boosted1_5[2], 128 - (int)(50 * m_boost_factor_p_6));
}

void SonicBoosterTest::unsigned16Data() {
  QAudioBuffer buffer = getBuffer<quint16>();

  quint16* raw_data = (quint16*)buffer.data();
  raw_data[0] = (1 << 15);
  raw_data[1] = (1 << 15) + 200;
  raw_data[2] = (1 << 15) - 200;

  QVERIFY(m_booster_m_6->boost(buffer));
  QVERIFY(m_booster_p_6->boost(buffer));

  int len0_5 = 0;
  const quint16* boosted0_5 = (quint16*)m_booster_m_6->getBoostedBuffer(len0_5);
  QCOMPARE(len0_5, 6);
  QCOMPARE((int)boosted0_5[0], (1 << 15));
  QCOMPARE((int)boosted0_5[1], (1 << 15) + (int)(200 * m_boost_factor_m_6));
  QCOMPARE((int)boosted0_5[2], (1 << 15) - (int)(200 * m_boost_factor_m_6));

  int len1_5 = 0;
  const quint16* boosted1_5 = (quint16*)m_booster_p_6->getBoostedBuffer(len1_5);
  QCOMPARE(len1_5, 6);
  QCOMPARE((int)boosted1_5[0], (1 << 15));
  QCOMPARE((int)boosted1_5[1], (1 << 15) + (int)(200 * m_boost_factor_p_6));
  QCOMPARE((int)boosted1_5[2], (1 << 15) - (int)(200 * m_boost_factor_p_6));
}
