#ifndef TYPINGTIMELORDTEST_H
#define TYPINGTIMELORDTEST_H

#include <QtTest>
#include <QSignalSpy>

#include <memory>

#include "typingtimelord.h"
#include "audioplayer.h"

class TypingTimeLordTest : public QObject {
  Q_OBJECT

public:
  TypingTimeLordTest();

private:
  std::shared_ptr<AudioPlayer> m_player;
  TypingTimeLord* m_time_lord;

private Q_SLOTS:
  void init();

  void keepPlayingWithNoTyping();
  void keepPlayingWithSlowTyping();
  void waitAndUnwait();
  void resetWaitTimerOnTypingPause();

  /** It should be able to set and retreive the timeout values, while
   *  keeping in bounds. */
  void setTimeouts();

  /** It shouldn't be possible to set a pause timeout value smaller than the
   *  typing timeout. */
  void dontSetTooSmallPauseTimeout();

  /** Test is changes in timeouts are actually taken into account (this is
   *  already implicitely tested in other tests). */
  void changeTimeouts();
};

#endif // TYPINGTIMELORDTEST_H
