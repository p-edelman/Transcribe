#include <QApplication>

#include "audioplayertest.h"
#include "typingtimelordtest.h"
#include "keycatchertest.h"
#include "transcribetest.h"

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  QTest::qExec(new AudioPlayerTest(), argc, argv);
  QTest::qExec(new TypingTimeLordTest(), argc, argv);
  QTest::qExec(new KeyCatcherTest(), argc, argv);
  QTest::qExec(new TranscribeTest(), argc, argv);

 return 0;
}
