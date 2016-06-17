#include "tst_keycatchertest.h"

KeyCatcherTest::KeyCatcherTest() {
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
