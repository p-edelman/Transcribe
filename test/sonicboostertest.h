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

  SonicBooster* m_booster0_5;
  SonicBooster* m_booster1_5;

  template<class word_type> QAudioBuffer getBuffer();

private Q_SLOTS:
  /** Test if the factor can be set with the public slots. */
  void setFactor();

  void unsigned8Data();
  void signed8Data();
  void signed16Data();
  void unsigned16Data();

  //void adjustFactorForCapping();
};

#endif // SONICBOOSTERTEST_H
