#ifndef SONICBOOSTERTEST_H
#define SONICBOOSTERTEST_H

#include <QtTest>
#include <QObject>

#include <QAudioFormat>

#include <limits>

#include "sonicbooster.h"

class SonicBoosterTest : public QObject {
  Q_OBJECT

public:
  SonicBoosterTest();

private:
  // The path to the testing wav files
  QString m_noise_file, m_silence_file;

  // Two boosters, one for +6 dB and one for -6 dB (boost factros of roughly
  // two and a half
  SonicBooster* m_booster_m_6;
  SonicBooster* m_booster_p_6;

  // The accompanying boost factors
  qreal m_boost_factor_m_6;
  qreal m_boost_factor_p_6;

  template<class word_type> QAudioBuffer getBuffer();

private Q_SLOTS:
  void unsigned8Data();
  void signed8Data();
  void signed16Data();
  void unsigned16Data();

  //void adjustFactorForCapping();
};

#endif // SONICBOOSTERTEST_H
