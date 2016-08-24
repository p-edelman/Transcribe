#ifndef TYPINGTIMEKEEPER_H
#define TYPINGTIMEKEEPER_H

#include <QObject>
#include <QSettings>
#include <QTimer>

#include <memory>

#include "audioplayer.h"

/** The idea is that when the user keeps typing, the playback should wait until
 *  she stops typing before continuing. This class is used to figure out when
 *  to stop and start playing audio.
 *
 *  It uses two timers to do this:
 *  - a waiting timer to keep track of when it is time to wait
 *  - a typing timer to register when a user stops typing
 *
 *  Then the following conditions can be used:
 *
 *  The timers start out in the stopped state. When the user types a key into
 *  the text editor:
 *  - When PLAYING: start the waiting timer if it is not running yet (we don't
 *                  start the timer until the first key is pressed, so the
 *                  user can listen uninterrupted after pressing the play
 *                  button until she starts typing.
 *  - When PLAYING or WAITING: (re)start the typing timer
 *
 *  On waiting timer expiration (the audio has run for some time and the user
 *  is still typing):
 *  - When PLAYING: switch to WAITING state and stop the waiting timer
 *
 *  On typing timer expiration (the user has stopped typing):
 *  - When PLAYING: reset the waiting timer to 0
 *  - When WAITING: switch to PLAYING state
 *  - Additionally, and start the typing timer again to wait for the next
 *                  'abscence of typing'.
 *
 *  When the AudioPlayer goes to the PAUSED state, both timers are stopped.
 *  When the AudioPlayer goes to the WAITING state, the waiting timer is
 *  stopped.
 */
class TypingTimeLord : public QObject {
  Q_OBJECT

public:
  explicit TypingTimeLord(std::shared_ptr<AudioPlayer> player,
                          QObject* parent = 0);

  void setWaitTimeout(unsigned int timeout);
  void setTypeTimeout(unsigned int timeout);
  int getWaitTimeout() {return m_wait_timeout;}
  int getTypeTimeout() {return m_type_timeout;}

  /** Expose the timeout values to QML */
  Q_PROPERTY(int wait_timeout READ getWaitTimeout WRITE setWaitTimeout)
  Q_PROPERTY(int type_timeout READ getTypeTimeout WRITE setTypeTimeout)

  /** Constants for the timeout limits. */
  static const unsigned int WAIT_TIMEOUT_MIN = 2000;
  static const unsigned int WAIT_TIMEOUT_MAX = 30000;
  static const unsigned int TYPE_TIMEOUT_MIN = 500;
  static const unsigned int TYPE_TIMEOUT_MAX = 5000;
  Q_PROPERTY(unsigned int wait_timeout_min MEMBER WAIT_TIMEOUT_MIN CONSTANT)
  Q_PROPERTY(unsigned int wait_timeout_max MEMBER WAIT_TIMEOUT_MAX CONSTANT)
  Q_PROPERTY(unsigned int type_timeout_min MEMBER TYPE_TIMEOUT_MIN CONSTANT)
  Q_PROPERTY(unsigned int type_timeout_max MEMBER TYPE_TIMEOUT_MAX CONSTANT)

public slots:
  /** Signal the time keeper that a key has been typed into the editor.
   *  This does two things:
   *  - it starts the pause timer if it is not running and if the audio is in
   *    PLAYING state.
   *  - it resets the typing timer if the audio is in PLAYING state. */
  void keyTyped();

private:
  /** Cancel the pause timer if it is running and start it from the beginning.
   *  This will be ignored if the player is not in PLAYING state. */
  void restartWaitTimer();

  /** Cancel the typing timer if it is running and start it from the beginning.
   *  This will be ignored if the player is not in PLAYING of WAITING state. */
  void restartTypeTimer();

  /** The intervals for the two timers, initialized to default values of 5 and
   *  1 seconds. */
  unsigned int m_wait_timeout = 5000;
  unsigned int m_type_timeout = 1000;

  QTimer m_wait_timer;
  QTimer m_type_timer;

  /** The AudioPlayer that we're controlling. */
  std::shared_ptr<AudioPlayer> m_player;

  /** The keys for the entries in the configuration file. */
  const QString CFG_GROUP   = "typing";
  const QString CFG_WAITING = "waiting";
  const QString CFG_TYPING  = "typing";

private slots:
  /** Callback for when the pause timer expires. */
  void waitTimeout();

  /** Callback for when the typing timer expires. */
  void typeTimeout();

  /** Callback for when the AudioPlayer has changed state. */
  void playerStateChanged();
};

#endif // TYPINGTIMEKEEPER_H
