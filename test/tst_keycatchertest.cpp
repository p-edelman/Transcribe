#include "tst_keycatchertest.h"

KeyCatcherTest::KeyCatcherTest() {
  m_root    = new QObject();
  m_catcher = new KeyCatcher(m_root);
  m_root->installEventFilter(m_catcher);
}

