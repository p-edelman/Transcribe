#include "transcribe.h"

Transcribe::Transcribe(int& argc, char** argv) : QGuiApplication(argc, argv) {
  m_player = new AudioPlayer();
}

void Transcribe::guiReady(QObject* root) {
  QObject* controls = root->findChild<QObject *>("media_controls");
  if (controls == NULL) {
    qDebug() << "Couldn't find the media player";
    // TODO: Handle this
  } else {
    m_player->setAudioControls(controls);
  }

  QObject* audio_chooser = root->findChild<QObject*>("audio_file_chooser");
  if (audio_chooser == NULL) {
    qDebug() << "Couldn't find the file chooser";
    // TODO: Handle this
  } else {
    QObject::connect(audio_chooser, SIGNAL(audioFileOpenendSignal(QString)),
                     m_player, SLOT(openAudioFile(QString)));
  }
}
