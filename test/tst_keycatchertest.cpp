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

/** Test if the Ctrl+S combination emits the saveFile() signal. Other modifiers
 *  should prevent the signal from being emitted. */
void KeyCatcherTest::testCtrlS() {
  QSignalSpy spy(m_catcher, SIGNAL(saveFile()));
  QKeyEvent* event = new QKeyEvent(QEvent::KeyPress, Qt::Key_S,
                                   Qt::ControlModifier);
  QApplication::sendEvent(m_root, event);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(m_key_typed_spy->count(), 0);

  delete event;
  event = new QKeyEvent(QEvent::KeyPress, Qt::Key_S,
                        Qt::ControlModifier & Qt::ShiftModifier);
  QApplication::sendEvent(m_root, event);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(m_key_typed_spy->count(), 1);

  delete event;
  event = new QKeyEvent(QEvent::KeyPress, Qt::Key_S,
                        Qt::ControlModifier & Qt::AltModifier);
  QApplication::sendEvent(m_root, event);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(m_key_typed_spy->count(), 2);

  delete event;
  event = new QKeyEvent(QEvent::KeyPress, Qt::Key_S,
                        Qt::AltModifier);
  QApplication::sendEvent(m_root, event);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(m_key_typed_spy->count(), 3);

  delete event;
}
