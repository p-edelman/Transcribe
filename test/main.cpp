#include <QCoreApplication>

#include "audioplayertest.h"
#include "typingtimelordtest.h"
#include "keycatchertest.h"

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  QTest::qExec(new AudioPlayerTest(), argc, argv);
  QTest::qExec(new TypingTimeLordTest(), argc, argv);
  QTest::qExec(new KeyCatcherTest(), argc, argv);

 return 0;
}
