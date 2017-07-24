#ifndef HISTORYMODELTEST_H
#define HISTORYMODELTEST_H

#include <QObject>

#include <QString>
#include <QStringList>
#include <QtTest>

#include "historymodel.h"

class HistoryModelTest : public QObject {
  Q_OBJECT

public:
  HistoryModelTest(QObject* parent = 0);

private Q_SLOTS:
  void init();

  /** On initialization, the history should be restored from the config file,
   *  but non-existing text files should be removed. */
  void saveAndRestore();

  /** It should be possible to add an item. It should be on the top of the list
   *  afterwards. The newly added item should be saved to the config file. */
  void addItem();

  /** If an item which is not on the first position is added again, it should
   *  rise to the top of the list. */
  void refreshItem();

  /** If the timestamp on an entry is changed, it should overwrite the previous
   *  timestamp. */
  void refreshTime();

  /** If an item with a given text file already exists but is added again with
   *  a different audio file, the item should be replaced with the new audio
   *  file. */
  void changeAudioFile();

  /** Wrongly formatted entries in the config file should be ignored. */
  void wronglyFormatted();

private:
  const QString CFG_GROUP = "history";

  QString m_text_file1;  // text1.txt
  QString m_text_file2;  // text2.txt
  QString m_text_none;   // none existing
  QString m_audio_file1; // audio1.wav (none existing, not actaully needed)
  QString m_audio_file2; // audio2.wav (none existing, not actaully needed)
  QString m_audio_file3; // audio3.wav (none existing, not actaully needed)
};

#endif // HISTORYMODELTEST_H
