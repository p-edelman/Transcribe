#ifndef TST_TYPINGTIMELORDTEST_H
#define TST_TYPINGTIMELORDTEST_H

#include <QtTest>
#include <QSignalSpy>

#include "typingtimelord.h"
#include "audioplayer.h"

class TypingTimeLordTest : public QObject {
  Q_OBJECT

public:
  TypingTimeLordTest();

private:
  AudioPlayer*    m_player;
  TypingTimeLord* m_time_lord;

private Q_SLOTS:
  void init();

  void keepPlayingWithNoTyping();
  void keepPlayingWithSlowTyping();
  void waitAndUnwait();
  void resetWaitTimerOnTypingPause();
};

#endif // TST_TYPINGTIMELORDTEST_H
