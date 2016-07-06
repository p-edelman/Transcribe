#ifndef TRANSCRIBETEST_H
#define TRANSCRIBETEST_H

#include <QtTest>
#include <QApplication>
#include <QFile>
#include <QIODevice>
#include <QObject>
#include <QQmlProperty>
#include <QString>
#include <QTextStream>
#include <QVariant>

#include "transcribe.h"

class TranscribeTest : public QObject {
  Q_OBJECT

public:
  explicit TranscribeTest(QObject *parent = 0);

private:
  Transcribe* m_transcribe;
  QObject*    m_app_window;

private Q_SLOTS:
  void init();
  void cleanup();

  /** If a non-existing text file is openend, it should be created. */
  void openNewTextFile();

  /** If an existing text file is opened, it should be loaded into the editor
   *  window and the file should remain intact. */
  void openExistingTextFile();

  /** If text is changed, the dirty flag should be set. If the file is saved,
   *  the changes should be committed to the file and the dirty flag should be
   *  cleared. */
  void saveTextChanges();

  /** When a text file is loaded, and an audio file is opened, the text file
   *  should be unloaded. */
  void unloadTextFileOnAudioOpening();
};

#endif // TRANSCRIBETEST_H
