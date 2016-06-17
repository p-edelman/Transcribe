#include <QCoreApplication>

#include "audioplayertest.h"
#include "typingtimelordtest.h"
#include "keycatchertest.h"

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  QTest::qExec(new AudioPlayerTest(), argc, argv);
  QTest::qExec(new TypingTimeLordTest(), argc, argv);
  QTest::qExec(new KeyCatcherTest(), argc, argv);

  // TODO: Find a way to test the main Transcribe class. As it is built now,
  // it is very hard to test as large part of its basic functionality is driven
  // through its GUI. It instantiates this GUI itself, making it hard to get
  // a grip on it.

 return 0;
}
