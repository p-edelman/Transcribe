#ifndef TST_KEYCATCHERTEST_H
#define TST_KEYCATCHERTEST_H

#include <QtTest>
#include <QSignalSpy>

#include "keycatcher.h"
#include "transcribe.h"

class KeyCatcherTest : public QObject {
  Q_OBJECT

public:
  KeyCatcherTest();

private:
  QObject*    m_root;
  KeyCatcher* m_catcher;
  QSignalSpy* m_key_typed_spy;

private Q_SLOTS:
  void init();

  void testCtrlS();
};

#endif // TST_KEYCATCHERTEST_H
