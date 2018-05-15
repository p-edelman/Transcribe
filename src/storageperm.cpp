#include "storageperm.h"

StoragePerm* StoragePerm::s_instance = 0;

StoragePerm::StoragePerm(QObject* parent) : QObject(parent) {
  s_instance = this;

  // This is actually a complicated way to call handleStoragePermResponse() from
  // receivedPermResponse(), but it is needed to break the lock on the
  // interface thread. If we call it directly, the GUI hangs.
  connect(this, SIGNAL(receivedPermResponse(bool)),
          this, SLOT(handleStoragePermResponse(bool)));
}

StoragePerm* StoragePerm::instance() {
  if (!s_instance) {
    s_instance = new StoragePerm();
  }
  return s_instance;
}

bool StoragePerm::tryPermission(std::function<void()> callback) {
  if (QtAndroid::checkPermission(s_instance->PERM) == QtAndroid::PermissionResult::Granted) {
    return true;
  }

  s_instance->callback = callback;
  s_instance->askForPermission();
  return false;
}

void StoragePerm::askForPermission() {
  QSettings settings;
  if (QtAndroid::shouldShowRequestPermissionRationale(PERM)) {
    // Show an explanation when we don't have permissions because the user
    // previously declined the request.
    showRationale();
  } else if (settings.value(CFG_PERMISSIONS_BLOCKED, false).toBool()) {
    // We can't complete because the permissions are permanently blocked.
    showError();
    return;
  }

  // Qt since version 5.10 has the storage request permissions built in, but
  // they seem very buggy and underdocumented, so we'll leave it to a dedicated
  // method in Java.
  QtAndroid::androidActivity().callMethod<void>("requestStoragePerm");
}

void StoragePerm::showRationale() {
  QMessageBox msg_box;
  msg_box.setWindowTitle("Permission needed");
  msg_box.setText("In order to open audio recordings, access to your files is needed.\n\nIf you deny this permission, playback will be disabled.");
  msg_box.setStandardButtons(QMessageBox::Ok);
  msg_box.exec();
}

void StoragePerm::showError() {
  QMessageBox msg_box;
  msg_box.setWindowTitle("Insufficient permissions");
  msg_box.setText("Audio playback is currently disabled.\n\nIf you want to play audio, please enable the 'Storage' permission.");
  msg_box.setStandardButtons(QMessageBox::Ok);
  msg_box.exec();
}

void StoragePerm::signalAndroidResponse(bool has_permission) {
  emit receivedPermResponse(has_permission);
}

void StoragePerm::handleStoragePermResponse(bool has_permission) {
  if (has_permission) {
    if (callback != 0) callback();
  } else {
    if (QtAndroid::shouldShowRequestPermissionRationale(PERM)) {
      askForPermission();
    } else {
      // We're in trouble, the user has permanently declined permission
      QSettings settings;
      settings.setValue(CFG_PERMISSIONS_BLOCKED, true);
      settings.sync();

      showError();
    }
  }
}

JNIEXPORT void JNICALL Java_org_mrpi_transcribe_TranscribeActivity_handleStorageRequestResponse(JNIEnv*, jobject, jboolean has_permisson) {
  StoragePerm::instance()->signalAndroidResponse(has_permisson);
}
