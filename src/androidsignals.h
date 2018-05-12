#ifndef SIGNALSFROMANDROID_H
#define SIGNALSFROMANDROID_H

#include <QObject>
#include <jni.h>

/** Call Java/Android methods from C++ is pretty straightforward, but the other
 *  way around is more involved and has several moving parts.
 *  - First a Java method should be added from C to an existing Java class with
 *    the JNI interface.
 *    - This method should be defined at the end of this file and implemented in
 *      the cpp file
 *    - And it should also be defined 'native' in the Java file.
 *  - Then there's the AndroidSignals class as a singleton, so that it
 *    can convert stateless events to the application state.
 *    - The Java-in-C method can now get a handle on the instance and call some
 *      method here to make it emit some signal
 *    - The C++ side of the application can also get a handle on the instance
 *      to catch the resulting signals.
 *
 *  So a call to the Java method from within Java should trigger a call to an
 *  AndroidSignals method, which should triggers a signal, which can be caught
 *  in the application. */
class AndroidSignals : public QObject {
  Q_OBJECT

private:
  static AndroidSignals* s_instance;
  explicit AndroidSignals(QObject *parent = 0);

public:
  static AndroidSignals* instance();

  /** Used to handle the response to an Android popup for requesting storage
   *  permissions. */
  void handleStoragePermResponse(bool has_permission);

signals:
  /** Sent when the user has responded to the permission request dialog.
   *  @param has_permission indicate if the permission was given. */
  void storagePermResponse(bool has_permission);
};

extern "C" {
  /** Handle the user response to the storage permission popup. */
  JNIEXPORT void JNICALL Java_org_mrpi_transcribe_TranscribeActivity_handleStorageRequestResponse(JNIEnv*, jobject, jboolean has_permisson);
}

#endif // SIGNALSFROMANDROID_H
