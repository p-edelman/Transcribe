#include <QApplication>

#include "transcribe.h"
#include "audioplayer.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // By setting these names on the QApplication, all classes can instantiate the
  // a QSettings object without arguments.
  app.setOrganizationName("MrPi");
  app.setOrganizationDomain("mrpi.org");
  app.setApplicationName("Transcribe");

  Transcribe transcribe;

  return app.exec();
}
