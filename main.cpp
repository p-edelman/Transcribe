#include <QQmlApplicationEngine>

#include "transcribe.h"
#include "audioplayer.h"

int main(int argc, char *argv[]) {
  Transcribe app(argc, argv);

  return app.exec();
}
