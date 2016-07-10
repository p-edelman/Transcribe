#ifndef TRANSCRIBETEST_H
#define TRANSCRIBETEST_H

#include <QtTest>
#include <QApplication>
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
  QString     m_tmp_dir;

  /** Used in conjunction with the dismissMessageBox() slot; if the error
   *  dialog was dismissed, this flag is set to true. */
  bool m_is_msg_box_dismissed = false;

public slots:
  /** This slot can be used with a QTimter::singleShot to dismiss all message
   *  boxes. If any message boxes are closed, m_is_msg_box_dismissed is set to
   *  true. */
  void dismissMessageBox();

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

  /** If the text file cannot be saved - in this case because it doesn't have
   *  write permissions - a message box should be displayed notifying the user
   *  and the dirty flag should remain set. */
  void unwritableTextFile();

  /** When a text file is loaded, and an audio file is opened, the text file
   *  should be unloaded. */
  void unloadTextFileOnAudioOpening();
};

#endif // TRANSCRIBETEST_H
