#include "tst_keycatchertest.h"

KeyCatcherTest::KeyCatcherTest() {
  qRegisterMetaType<AudioPlayer::SeekDirection>();

  m_root    = new QObject();
  m_catcher = new KeyCatcher(m_root);
  m_root->installEventFilter(m_catcher);

  m_key_typed_spy = new QSignalSpy(m_catcher, SIGNAL(keyTyped()));
}

void KeyCatcherTest::init() {
  m_key_typed_spy->clear();
}

/** Return a list of all the key modifier combinations that shouldn't trigger
 *  the action. */
QList<Qt::KeyboardModifiers> KeyCatcherTest::getInvalidModifiers(QList<Qt::KeyboardModifiers>& valid_modifiers) {
  QList<Qt::KeyboardModifiers> modifiers;
  modifiers.append(Qt::ControlModifier);
  modifiers.append(Qt::AltModifier);
  modifiers.append(Qt::ShiftModifier);
  modifiers.append(Qt::MetaModifier);
  modifiers.append(Qt::ControlModifier | Qt::AltModifier);
  modifiers.append(Qt::ControlModifier | Qt::ShiftModifier);
  modifiers.append(Qt::ControlModifier | Qt::MetaModifier);
  modifiers.append(Qt::AltModifier | Qt::ShiftModifier);
  modifiers.append(Qt::AltModifier | Qt::MetaModifier);
  modifiers.append(Qt::ShiftModifier | Qt::MetaModifier);
  modifiers.append(Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier);
  modifiers.append(Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);
  modifiers.append(Qt::ControlModifier | Qt::ShiftModifier | Qt::MetaModifier);
  modifiers.append(Qt::AltModifier | Qt::ShiftModifier | Qt::MetaModifier);
  modifiers.append(Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier | Qt::MetaModifier);

  QList<Qt::KeyboardModifiers>::iterator mod;
  for (mod = valid_modifiers.begin(); mod != valid_modifiers.end(); mod++) {
    modifiers.removeAll(*mod);
  }

  return modifiers;
}

/** Test if the Ctrl+S combination emits the saveFile() signal. Other modifiers
 *  should prevent the signal from being emitted. */
void KeyCatcherTest::testCtrlS() {
  QList<Qt::KeyboardModifiers> valid_modifiers;
  valid_modifiers.append(Qt::ControlModifier);

  QSignalSpy spy(m_catcher, SIGNAL(saveFile()));

  QList<Qt::KeyboardModifiers>::iterator vmod;
  for (vmod = valid_modifiers.begin(); vmod != valid_modifiers.end(); vmod++) {
    QKeyEvent event(QEvent::KeyPress, Qt::Key_S, *vmod);
    QApplication::sendEvent(m_root, &event);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_key_typed_spy->count(), 0);
  }

  QList<Qt::KeyboardModifiers> invalid_modifiers = getInvalidModifiers(valid_modifiers);
  int key_typed = 0;
  QList<Qt::KeyboardModifiers>::iterator imod;
  for (imod = invalid_modifiers.begin(); imod != invalid_modifiers.end(); imod++) {
    QKeyEvent event(QEvent::KeyPress, Qt::Key_S, *imod);
    QApplication::sendEvent(m_root, &event);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_key_typed_spy->count(), ++key_typed);
  }
}

/** Test if the audio playing and pausing can be controlled with Ctrl+Space. */
void KeyCatcherTest::testAudioPlayPauseWithSpace() {
  QList<Qt::KeyboardModifiers> valid_modifiers;
  valid_modifiers.append(Qt::ControlModifier);
  QList<Qt::KeyboardModifiers> invalid_modifiers = getInvalidModifiers(valid_modifiers);

  QSignalSpy spy(m_catcher, SIGNAL(togglePlayPause()));

  QList<Qt::KeyboardModifiers>::iterator vmod;
  for (vmod = valid_modifiers.begin(); vmod != valid_modifiers.end(); vmod++) {
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Space, *vmod);
    QApplication::sendEvent(m_root, &event);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_key_typed_spy->count(), 0);
  }

  // Test if other modifiers are ignored
  int key_typed = 0;
  QList<Qt::KeyboardModifiers>::iterator imod;
  for (imod = invalid_modifiers.begin(); imod != invalid_modifiers.end(); imod++) {
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Space, *imod);
    QApplication::sendEvent(m_root, &event);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_key_typed_spy->count(), ++key_typed);
  }
}

/** Test is we can seek the audio with Alt+Left and Alt+Right. */
void KeyCatcherTest::testAudioSeekWithArrows() {
  QList<Qt::KeyboardModifiers> valid_modifiers;
  valid_modifiers.append(Qt::AltModifier);
  QList<Qt::KeyboardModifiers> invalid_modifiers = getInvalidModifiers(valid_modifiers);

  QSignalSpy spy(m_catcher, SIGNAL(seekAudio(AudioPlayer::SeekDirection, int)));

  QList<Qt::KeyboardModifiers>::iterator vmod;
  for (vmod = valid_modifiers.begin(); vmod != valid_modifiers.end(); vmod++) {
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Left, *vmod);
    QApplication::sendEvent(m_root, &event);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.last().at(0), QVariant(AudioPlayer::BACKWARD));
    QCOMPARE(m_key_typed_spy->count(), 0);
  }

  for (vmod = valid_modifiers.begin(); vmod != valid_modifiers.end(); vmod++) {
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Right, *vmod);
    QApplication::sendEvent(m_root, &event);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.last().at(0), QVariant(AudioPlayer::FORWARD));
    QCOMPARE(m_key_typed_spy->count(), 0);
  }

  // Test if other modifiers are ignored
  int key_typed = 0;
  QList<Qt::KeyboardModifiers>::iterator imod;
  for (imod = invalid_modifiers.begin(); imod != invalid_modifiers.end(); imod++) {
    QKeyEvent event_left(QEvent::KeyPress, Qt::Key_Left, *imod);
    QApplication::sendEvent(m_root, &event_left);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(m_key_typed_spy->count(), ++key_typed);
    QKeyEvent event_right(QEvent::KeyPress, Qt::Key_Right, *imod);
    QApplication::sendEvent(m_root, &event_right);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(m_key_typed_spy->count(), ++key_typed);
  }
}

/** Test if we can control the audio stopping and pausing with the hardware
 *  audio keys. */
void KeyCatcherTest::testAudioPlayPauseWithAudioKeys() {
  QSignalSpy spy_noarg(m_catcher, SIGNAL(togglePlayPause()));
  QSignalSpy spy_arg(m_catcher, SIGNAL(togglePlayPause(bool)));

  QApplication::sendEvent(m_root,
    new QKeyEvent(QEvent::KeyPress, Qt::Key_MediaTogglePlayPause, 0));
  QCOMPARE(spy_noarg.count(), 1);
  QCOMPARE(spy_arg.count(), 0);

  // TODO: We now use MediaPlay as a toggle as this seems to be the correct
  // interpretation, but we should look into this.
  QApplication::sendEvent(m_root,
    new QKeyEvent(QEvent::KeyPress, Qt::Key_MediaPlay, 0));
  QCOMPARE(spy_noarg.count(), 2);
  QCOMPARE(spy_arg.count(), 0);

  QApplication::sendEvent(m_root,
    new QKeyEvent(QEvent::KeyPress, Qt::Key_MediaPause, 0));
  QCOMPARE(spy_noarg.count(), 2);
  QCOMPARE(spy_arg.count(), 1);
  QCOMPARE(spy_arg.last().at(0).toBool(), false);

  QApplication::sendEvent(m_root,
    new QKeyEvent(QEvent::KeyPress, Qt::Key_MediaStop, 0));
  QCOMPARE(spy_noarg.count(), 2);
  QCOMPARE(spy_arg.count(), 2);
  QCOMPARE(spy_arg.last().at(0).toBool(), false);
}

/** Test if we can seek through the audio with the hardware previous and next
 *  keys. */
void KeyCatcherTest::testAudioSeekWithAudioKeys() {
  QSignalSpy spy(m_catcher, SIGNAL(seekAudio(AudioPlayer::SeekDirection,int)));

  QApplication::sendEvent(m_root,
    new QKeyEvent(QEvent::KeyPress, Qt::Key_MediaPrevious, 0));
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.last().at(0), QVariant(AudioPlayer::BACKWARD));

  QApplication::sendEvent(m_root,
    new QKeyEvent(QEvent::KeyPress, Qt::Key_MediaNext, 0));
  QCOMPARE(spy.count(), 2);
  QCOMPARE(spy.last().at(0), QVariant(AudioPlayer::FORWARD));
}

/** Test if modifiers on the hardware audio keys are ignored. */
void KeyCatcherTest::testModifiersOnAudioAudioKeys() {
  QSignalSpy spy_pp_noarg(m_catcher, SIGNAL(togglePlayPause()));
  QSignalSpy spy_pp_arg(m_catcher, SIGNAL(togglePlayPause(bool)));
  QSignalSpy spy_seek(m_catcher, SIGNAL(seekAudio(AudioPlayer::SeekDirection,int)));
  QList<Qt::KeyboardModifiers> mods;
  QList<Qt::KeyboardModifiers> invalid_mods = getInvalidModifiers(mods);

  QList<Qt::Key> audio_keys;
  audio_keys.append(Qt::Key_MediaPlay);
  audio_keys.append(Qt::Key_MediaTogglePlayPause);
  audio_keys.append(Qt::Key_MediaStop);
  audio_keys.append(Qt::Key_MediaPause);
  audio_keys.append(Qt::Key_MediaPrevious);
  audio_keys.append(Qt::Key_MediaNext);

  int key_typed = 0;
  QList<Qt::Key>::iterator audio_key;
  QList<Qt::KeyboardModifiers>::iterator imod;
  for (audio_key = audio_keys.begin(); audio_key != audio_keys.end(); audio_key++) {
    for (imod = invalid_mods.begin(); imod != invalid_mods.end(); imod++) {
      QKeyEvent event(QEvent::KeyPress, *audio_key, *imod);
      QApplication::sendEvent(m_root, &event);
      QCOMPARE(spy_pp_noarg.count(), 0);
      QCOMPARE(spy_pp_arg.count(), 0);
      QCOMPARE(spy_seek.count(), 0);
      // Every key that isn't consumed is regarded as normal typing
      QCOMPARE(m_key_typed_spy->count(), ++key_typed);
    }
  }
}
