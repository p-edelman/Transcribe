#ifndef STORAGEPERM_H
#define STORAGEPERM_H

#include <QObject>

#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

#include <QDebug>
#include <QtAndroid>
#include <jni.h>

/** Class to handle the interaction with Androids permission model for the
 *  general storage.
 *
 *  There are quite a few things to take into account. First of all, the
 *  permission model:
 *  - Android will popup a message requesting the neccessery permissions when
 *    they are first needed. The user can choose to accept or decline.
 *  - If the user accepts, the program should resume where it left off.
 *  - If the user declines, the program can show an explanation and ask again.
 *  - Now the user has the choice to accept, decline again, or decline and
 *    never ask again.
 *  - If the user simple declines, the circle starts again
 *  - If the user blocks, this should be handled gracefully.
 *
 *  Note that Android can tell the program when an extra exeplanation is needed,
 *  but you cannot know whether the permission is blocked - just if you have it
 *  or not.
 *
 *  Then there is the Java complication. Qt since version 5.10 supports the
 *  Android mechanisms to request and query permissions, but it is buggy. So
 *  we have to call some native Android to make this work, which in the case
 *  of permission requesting is asynchronous.
 *  To make this work, we need several moving parts.
 *  - First the Java method handleStorageRequestResponse() is added from C to
 *    the main Java Activity class with the JNI interface. It is defined here,
 *    implemented in the .cpp file and then declared 'native' in the Java file.
 *  - Second, this class is a singleton, so that it can convert stateless events
 *    to the application state.
 *  - The Java-in-C method can now get a handle on the instance and call the
 *    signalAndroidResponse() method, which in turn triggers the emission of the
 *    receivedPermResponse() signal.
 *  - Now this class can catch this signal and respond to it. (The signal/slot
 *    mechanism is necessary to not let the main loop hang).
 */
class StoragePerm : public QObject {
  Q_OBJECT

public:
  static StoragePerm* instance();

  /** Return true if the app has the necessary permissions. It the permissions
   *  aren't given, return false but start the asynchronous procedure to obtain
   *  them. When the permissions are succesfully obtained, callback will be
   *  called. */
  bool tryPermission(std::function<void()> callback);

  /** Send the receivedPermResponse() signal with the specified answer. */
  void signalAndroidResponse(bool has_permission);

private:
  // The name of the permission that we need
  const QString PERM = "android.permission.READ_EXTERNAL_STORAGE";

  // A configuration setting so that we can check if the user has blocked
  // the permission definitely.
  const QString CFG_PERMISSIONS_BLOCKED = "is_permissions_blocked";

  // A callback that we can call on getting the permissions.
  std::function<void()> callback = 0;

  static StoragePerm* s_instance;
  explicit StoragePerm(QObject *parent = 0);

  /** Present the user with a popup to ask for permission (when it's not blocked
   *  at least). This part is handed over to Android. */
  void askForPermission();

  /** Show a box to explain to the user why the permissions are needed. */
  void showRationale();

  /** Show an error box that audio loading is disable because of insufficient
   *  permissions. */
  void showError();

private slots:
  /** Used to handle the response to an Android popup for requesting storage
   *  permissions. */
  void handleStoragePermResponse(bool has_permission);

signals:
  /** Sent when we've got a response from the user on our permissions request
   *  popup. */
  void receivedPermResponse(bool);
};

extern "C" {
  /** Handle the user response to the storage permission popup. */
  JNIEXPORT void JNICALL Java_org_mrpi_transcribe_TranscribeActivity_handleStorageRequestResponse(JNIEnv*, jobject, jboolean has_permisson);
}

#endif // STORAGEPERM_H
