#ifndef KEYCATCHER_H
#define KEYCATCHER_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>

#include "audioplayer.h"

/** Catches keys from the application. */
class KeyCatcher : public QObject {
  Q_OBJECT

public:
  KeyCatcher(QObject* parent = 0);

signals:
  /** Emitted when a key is typed into the editor. Control keys, like the media
      keys or the menu shortcuts don't result in this signal. */
  void keyTyped();

  /** Signals for audio control */
  void seekAudio(AudioPlayer::SeekDirection, int seconds);
  void togglePlayPause();
  void togglePlayPause(bool should_play);

  /** Emitted if a key combination is typed that signals that the file should
   *  be saved. */
  void saveFile();

protected:
  /** The raise d'etre of this class: catching keys. */
  bool eventFilter(QObject* object, QEvent* event);
};

#endif // KEYPRESSEATER_H
