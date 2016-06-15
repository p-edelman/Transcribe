#ifndef KEYCATCHER_H
#define KEYCATCHER_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>

#include "audioplayer.h"
#include "typingtimelord.h"

/** Catches keys from the application. */
class KeyCatcher : public QObject {
  Q_OBJECT

public:
  /** Initialize the KeyCatcher.
   *  @param transcribe the main Transcribe object with with the KeyCatcher
   *                    will communicate. */
  KeyCatcher(AudioPlayer* audio_player,
             QObject*     parent = 0);

signals:
  /** Emitted when a key is typed into the editor. Control keys, like the media
      keys or the menu shortcuts don't result in this signal. */
  void keyTyped();

  /** Emitted if a key combination is typed that signals that the file should
   *  be saved. */
  void saveFile();

protected:
  /** The raise d'etre of this class: catching keys. */
  bool eventFilter(QObject* object, QEvent* event);

private:
  /** The AudioPlayer associated with the transcribe object. */
  AudioPlayer* m_player;
};

#endif // KEYPRESSEATER_H
