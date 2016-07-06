#include "transcribetest.h"

TranscribeTest::TranscribeTest(QObject* parent) : QObject(parent) {}

void TranscribeTest::init() {
  int argc = 0;
  m_transcribe = new Transcribe(argc, NULL);
  m_app_window = QApplication::allWindows().at(0);
}

void TranscribeTest::cleanup() {
  delete m_transcribe;
}

void TranscribeTest::openNewTextFile() {
  QString file_name = "new.txt";

  QFile file(file_name);
  if (file.exists()) {
    file.remove();
  }

  m_transcribe->openTextFile(file_name);

  QVERIFY(file.exists());
  QCOMPARE(file.size(), 0);
  QCOMPARE(m_transcribe->getTextFileName(), file_name);

  file.remove();
}

void TranscribeTest::openExistingTextFile() {
  // Prepare a text file
  QString file_name = "existing.txt";
  QFile file(file_name);
  if (file.exists()) {
    QFAIL("File already exists");
  }
  QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
  QTextStream out_stream(&file);
  QString initial_text = "Initial text";
  out_stream << initial_text;
  file.close();
  int orig_size = file.size();

  // Open the text file
  m_transcribe->openTextFile(file_name);

  // Compare the text in the editor window
  QObject* text_area = m_app_window->findChild<QObject*>("text_area");
  QVariant text_area_cont = QQmlProperty::read(text_area, "text");
  QCOMPARE(text_area_cont.toString(), initial_text);

  // Check if the file is kept intact
  QVERIFY(file.exists());
  QCOMPARE(file.size(), orig_size);
  QCOMPARE(m_transcribe->getTextFileName(), file_name);

  file.remove();
}


void TranscribeTest::saveTextChanges() {
  QString file_name = "new.txt";

  // Open an empty text file
  m_transcribe->openTextFile(file_name);
  QCOMPARE(m_transcribe->isTextDirty(), false);

  // Write something to the editor window
  QObject* text_area = m_app_window->findChild<QObject*>("text_area");
  QString hello_world = "Hello world!";
  QVariant editor_pos(0);
  QVariant editor_text(hello_world);
  QMetaObject::invokeMethod(text_area, "insert", QGenericReturnArgument(),
                            Q_ARG(QVariant, editor_pos),
                            Q_ARG(QVariant, editor_text));

  // Check if dirty flag is set.
  QCOMPARE(m_transcribe->isTextDirty(), true);

  // Now save the text. Dirty flag should be cleared, and the text file should
  // contain the altered text.
  m_transcribe->saveText();
  QCOMPARE(m_transcribe->isTextDirty(), false);

  QFile file(file_name);
  file.open(QIODevice::ReadOnly | QIODevice::Text);
  QTextStream in_stream(&file);
  QString file_text(in_stream.readAll());
  QCOMPARE(file_text, hello_world);

  file.remove();
}

void TranscribeTest::unloadTextFileOnAudioOpening() {
  QString file_name = "new.txt";
  m_transcribe->openTextFile(file_name);
  QCOMPARE(m_transcribe->getTextFileName(), file_name);

  QString silence_file = QString(SRCDIR);
  silence_file += "files/silence.wav";
  m_transcribe->openAudioFile(silence_file);

  QCOMPARE(m_transcribe->getTextFileName(), QString("No transcript file loaded"));
}
