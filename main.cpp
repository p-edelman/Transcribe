#include <QQmlApplicationEngine>
#include <QtQml>

#include "transcribe.h"
#include "audioplayer.h"

int main(int argc, char *argv[]) {
  Transcribe app(argc, argv);

  // This is a bit of quirkiness of Qt; you can't declare an enum as a QML type,
  // but you can declare a C++ with a public enum as a QML type, and than
  // access the enum values as type properties. So we expose the
  // "AudioPlayer" (the class) as "PlayerState" in QML.
  qmlRegisterType<AudioPlayer>("AudioPlayer", 1, 0, "PlayerState");

  QQmlApplicationEngine engine;
  QObject::connect(&engine, SIGNAL(objectCreated(QObject*, QUrl)),
                   &app,    SLOT(guiReady(QObject*)));
  engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

  return app.exec();
}
