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

    // Keep track of whether we consume the key press or need to send it
    // along
    bool is_consumed = true;

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
        m_player->seek(SeekDirection::FORWARD, 5);
        break;
      case Qt::Key_MediaPrevious:
        m_player->seek(SeekDirection::BACKWARD, 5);
        break;
      case Qt::Key_Space:
        if (key_event->modifiers() == Qt::ControlModifier) {
          m_player->togglePlayPause();
        } else {
          is_consumed = false;
        }
        break;
      case Qt::Key_Left:
        if (key_event->modifiers() == Qt::AltModifier) {
          m_player->seek(SeekDirection::BACKWARD, 5);
        } else {
          is_consumed = false;
        }
        break;
      case Qt::Key_Right:
        if (key_event->modifiers() == Qt::AltModifier) {
          m_player->seek(SeekDirection::FORWARD, 5);
        } else {
          is_consumed = false;
        }
        break;
      case Qt::Key_S:
        if (key_event->modifiers() == Qt::ControlModifier) {
          m_transcribe->saveText();
        } else {
          is_consumed = false;
        }
        break;
      default:
        is_consumed = false;
    }

    // When the key is not consumed the user is typing in the window and
    // we should signal so.
    if (!is_consumed) emit keyTyped();

    return is_consumed;
  }

  return QObject::eventFilter(object, event);
}
