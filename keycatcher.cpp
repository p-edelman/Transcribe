#include "keycatcher.h"
#include "transcribe.h"

KeyCatcher::KeyCatcher(Transcribe*  transcribe,
                       AudioPlayer* audio_player,
                       QObject*     parent) : QObject(parent) {
  m_transcribe = transcribe;
  m_player     = audio_player;
}

bool KeyCatcher::eventFilter(QObject* object, QEvent* event) {
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent* key_event = static_cast<QKeyEvent*>(event);

    switch (key_event->key()) {
      case Qt::Key_MediaPlay:
      case Qt::Key_MediaTogglePlayPause:
        m_player->togglePlayPause();
        break;
      case Qt::Key_MediaStop:
      case Qt::Key_MediaPause:
        m_player->togglePlayPause(false);
        break;
      case Qt::Key_MediaNext:
        break;
      case Qt::Key_MediaPrevious:
        break;
    }

    return true;
  } else {
    return QObject::eventFilter(object, event);
  }
}
