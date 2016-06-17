#ifndef TST_KEYCATCHERTEST_H
#define TST_KEYCATCHERTEST_H

#include <QtTest>
#include <QSignalSpy>
#include <QList>
#include <QMetaType>

#include "keycatcher.h"
#include "transcribe.h"
#include "audioplayer.h"

Q_DECLARE_METATYPE(AudioPlayer::SeekDirection)

class KeyCatcherTest : public QObject {
  Q_OBJECT

public:
  KeyCatcherTest();

private:
  QObject*    m_root;
  KeyCatcher* m_catcher;
  QSignalSpy* m_key_typed_spy;

  QList<Qt::KeyboardModifiers> getInvalidModifiers(QList<Qt::KeyboardModifiers>& valid_modifiers);

private Q_SLOTS:
  void init();

  void testCtrlS();
  void testAudioPlayPauseWithSpace();
  void testAudioSeekWithArrows();
};

#endif // TST_KEYCATCHERTEST_H
