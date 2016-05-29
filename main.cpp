#include <QQmlApplicationEngine>

#include "transcribe.h"

int main(int argc, char *argv[]) {
  Transcribe app(argc, argv);

  QQmlApplicationEngine engine;
  QObject::connect(&engine, SIGNAL(objectCreated(QObject*, QUrl)),
                   &app,    SLOT(guiReady(QObject*)));
  engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

  return app.exec();
}
