#include "keycatcher.h"
#include "transcribe.h"

KeyCatcher::KeyCatcher(QObject* parent) : QObject(parent) {}

bool KeyCatcher::eventFilter(QObject* object, QEvent* event) {
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent* key_event = static_cast<QKeyEvent*>(event);

    // Keep track of whether we consume the key press or need to send it
    // along
    bool is_consumed = true;

    if (key_event->modifiers() == Qt::NoModifier) {
      switch(key_event->key()) {
        case Qt::Key_MediaPlay:
        case Qt::Key_MediaTogglePlayPause:
          emit togglePlayPause();
          break;
        case Qt::Key_MediaStop:
        case Qt::Key_MediaPause:
          emit togglePlayPause(false);
          break;
        case Qt::Key_MediaNext:
          emit seekAudio(AudioPlayer::FORWARD, 5);
          break;
        case Qt::Key_MediaPrevious:
          emit seekAudio(AudioPlayer::BACKWARD, 5);
          break;
        default:
          is_consumed = false;
      }
    } else if (key_event->modifiers() == Qt::ControlModifier) {
      switch(key_event->key()) {
        case Qt::Key_Space:
          emit togglePlayPause();
          break;
        case Qt::Key_S:
          emit saveFile();
          break;
        default:
          is_consumed = false;
      }
    } else if (key_event->modifiers() == Qt::AltModifier) {
      switch(key_event->key()) {
        case Qt::Key_Left:
          emit seekAudio(AudioPlayer::BACKWARD, 5);
          break;
        case Qt::Key_Right:
          emit seekAudio(AudioPlayer::FORWARD, 5);
          break;
        case Qt::Key_Up:
          emit boost(true);
          break;
        case Qt::Key_Down:
          emit boost(false);
          break;
        default:
          is_consumed = false;
      }
    } else {
      is_consumed = false;
    }

    // When the key is not consumed the user is typing in the window and
    // we should signal so.
    if (!is_consumed) emit keyTyped();

    return is_consumed;
  }

  return QObject::eventFilter(object, event);
}
