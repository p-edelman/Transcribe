#include "historymodeltest.h"

HistoryModelTest::HistoryModelTest(QObject *parent) : QObject(parent) {
  m_text_file1 =  QString(SRCDIR);
  m_text_file1 += "files/text1.txt";

  m_text_file2 =  QString(SRCDIR);
  m_text_file2 += "files/text2.txt";

  m_text_none =  QString(SRCDIR);
  m_text_none += "files/none.txt";

  m_audio_file1 = QString(SRCDIR);
  m_audio_file1 += "files/audio1.wav";

  m_audio_file2 = QString(SRCDIR);
  m_audio_file2 += "files/audio2.wav";

  m_audio_file3 = QString(SRCDIR);
  m_audio_file3 += "files/audio3.wav";
}

void HistoryModelTest::init() {
  // Remove all history from the settings file
  QSettings settings;
  settings.beginGroup(CFG_GROUP);
  for (QString key : settings.allKeys()) {
    settings.remove(key);
  }
  settings.sync();
}

void HistoryModelTest::saveAndRestore() {
  HistoryModel history1;
  history1.add(m_text_file1, m_audio_file1);
  history1.add(m_text_file2, m_audio_file2);
  history1.add(m_text_none,  m_audio_file3);

  HistoryModel history2;
  QCOMPARE(history2.rowCount(), 2);

  QModelIndex index = history2.index(0, 0);
  QCOMPARE(index.data(HistoryModel::TextFileRole).toString(),  m_text_file2);
  QCOMPARE(index.data(HistoryModel::AudioFileRole).toString(), m_audio_file2);

  index = history2.index(1, 0);
  QCOMPARE(index.data(HistoryModel::TextFileRole).toString(),  m_text_file1);
  QCOMPARE(index.data(HistoryModel::AudioFileRole).toString(), m_audio_file1);
}

void HistoryModelTest::addItem() {
  HistoryModel history1;

  // Add item
  history1.add(m_text_file1, m_audio_file1);
  QCOMPARE(history1.rowCount(), 1);
  HistoryModel history2;
  QCOMPARE(history2.rowCount(), 1);

  // Add another item
  history1.add(m_text_file2, m_audio_file2);
  HistoryModel history3;
  QCOMPARE(history3.rowCount(), 2);

  // Check if the order of items is correct
  QModelIndex index = history1.index(0, 0);
  QCOMPARE(index.data(HistoryModel::TextFileRole).toString(),  m_text_file2);
  QCOMPARE(index.data(HistoryModel::AudioFileRole).toString(), m_audio_file2);

  index = history1.index(1, 0);
  QCOMPARE(index.data(HistoryModel::TextFileRole).toString(),  m_text_file1);
  QCOMPARE(index.data(HistoryModel::AudioFileRole).toString(), m_audio_file1);
}

void HistoryModelTest::refreshItem() {
  HistoryModel history;
  history.add(m_text_file1, m_audio_file1);
  history.add(m_text_file2, m_audio_file2);
  history.add(m_text_file1, m_audio_file1);

  QCOMPARE(history.rowCount(), 2);

  QModelIndex index = history.index(0, 0);
  QCOMPARE(index.data(HistoryModel::TextFileRole).toString(),  m_text_file1);
  QCOMPARE(index.data(HistoryModel::AudioFileRole).toString(), m_audio_file1);

  index = history.index(1, 0);
  QCOMPARE(index.data(HistoryModel::TextFileRole).toString(),  m_text_file2);
  QCOMPARE(index.data(HistoryModel::AudioFileRole).toString(), m_audio_file2);
}

void HistoryModelTest::changeAudioFile() {
  HistoryModel history;
  history.add(m_text_file1, m_audio_file1);
  history.add(m_text_file2, m_audio_file2);
  history.add(m_text_file1, m_audio_file3);

  QCOMPARE(history.rowCount(), 2);

  QModelIndex index = history.index(0, 0);
  QCOMPARE(index.data(HistoryModel::TextFileRole).toString(),  m_text_file1);
  QCOMPARE(index.data(HistoryModel::AudioFileRole).toString(), m_audio_file3);

  index = history.index(1, 0);
  QCOMPARE(index.data(HistoryModel::TextFileRole).toString(),  m_text_file2);
  QCOMPARE(index.data(HistoryModel::AudioFileRole).toString(), m_audio_file2);
}

void HistoryModelTest::wronglyFormatted() {
  QSettings settings;
  settings.beginGroup(CFG_GROUP);

  // One file too many
  QStringList files;
  files.append(m_text_file1);
  settings.setValue(QString::number(0), files);

  // Proper amount of files, but no numeric key
  files.append(m_audio_file1);
  settings.setValue("Foo", files);

  // Proper entry
  settings.setValue(QString::number(2), files);

  // One file too many
  files.append(m_text_file2);
  settings.setValue(QString::number(3), files);

  HistoryModel history;
  QCOMPARE(history.rowCount(), 1);
}
