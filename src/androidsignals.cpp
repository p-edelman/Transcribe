#include "androidsignals.h"

AndroidSignals* AndroidSignals::s_instance = 0;

AndroidSignals::AndroidSignals(QObject *parent) : QObject(parent) {
  s_instance = this;
}

AndroidSignals* AndroidSignals::instance() {
  if (!s_instance) {
    s_instance = new AndroidSignals();
  }
  return s_instance;
}

void AndroidSignals::handleStoragePermResponse(bool has_response) {
  emit storagePermResponse(has_response);
}

JNIEXPORT void JNICALL Java_org_mrpi_transcribe_TranscribeActivity_handleStorageRequestResponse(JNIEnv*, jobject, jboolean has_permisson) {
  AndroidSignals::instance()->storagePermResponse(has_permisson);
}
