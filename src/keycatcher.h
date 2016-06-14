#ifndef KEYCATCHER_H
#define KEYCATCHER_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>

#include "audioplayer.h"
#include "typingtimelord.h"

class Transcribe; // Forward declaration because we would otherwise get a
                  // circular dependancy.

/** Catches keys from the application. */
class KeyCatcher : public QObject {
  Q_OBJECT

public:
  /** Initialize the KeyCatcher.
   *  @param transcribe the main Transcribe object with with the KeyCatcher
   *                    will communicate. */
  KeyCatcher(Transcribe*  transcribe,
             AudioPlayer* audio_player,
             QObject*     parent = 0);

signals:
  /** Emitted when a key is typed into the editor. Control keys, like the media
      keys or the menu shortcuts don't result in this signal. */
  void keyTyped();

protected:
  /** The raise d'etre of this class: catching keys. */
  bool eventFilter(QObject* object, QEvent* event);

private:
  /** The main transcribe object. */
  Transcribe*  m_transcribe;

  /** The AudioPlayer associated with the transcribe object. */
  AudioPlayer* m_player;
};

#endif // KEYPRESSEATER_H
