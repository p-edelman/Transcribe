#include <QQmlApplicationEngine>

#include "transcribe.h"

int main(int argc, char *argv[]) {
  Transcribe app(argc, argv);

  QQmlApplicationEngine engine;
  engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

  return app.exec();
}
