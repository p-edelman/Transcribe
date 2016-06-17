#include <QCoreApplication>

#include "tst_audioplayertest.h"
#include "tst_typingtimelordtest.h"

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  QTest::qExec(new AudioPlayerTest(), argc, argv);
  QTest::qExec(new TypingTimeLordTest(), argc, argv);

 return 0;
}
