#include "transcribetest.h"

TranscribeTest::TranscribeTest(QObject* parent) : QObject(parent) {
  m_tmp_dir = QString(SRCDIR) + "/tmp/";
}

void TranscribeTest::init() {
  // Create new Transcribe object
  int argc = 0;
  m_transcribe = new Transcribe(argc, NULL);
  m_app_window = QApplication::allWindows().at(0);

  m_is_msg_box_dismissed = false;
}

void TranscribeTest::cleanup() {
  delete m_transcribe;

  // Clear the tmd directory
  QDir tmp_dir(m_tmp_dir);
  for (QString file_name: tmp_dir.entryList(QDir::Files)) {
    QFile file(m_tmp_dir + file_name);
    file.setPermissions(QFileDevice::WriteOwner);
    file.remove();
  }
}

void TranscribeTest::dismissMessageBox() {
  QWidgetList widgets = QApplication::allWidgets();
  for (QWidget* widget: widgets) {
    QMessageBox* box = qobject_cast<QMessageBox*>(widget);
    if (box != NULL) {
      QTest::keyClick(box, Qt::Key_Enter);
      m_is_msg_box_dismissed = true;
    }
  }
}

void TranscribeTest::openNewTextFile() {
  QString file_name = "new.txt";

  QFile file(m_tmp_dir + file_name);
  if (file.exists()) {
    file.remove();
  }

  m_transcribe->openTextFile(m_tmp_dir + file_name);

  QVERIFY(file.exists());
  QCOMPARE(file.size(), 0);
  QCOMPARE(m_transcribe->getTextFileName(), file_name);

  file.remove();
}

void TranscribeTest::openExistingTextFile() {
  // Prepare a text file
  QString file_name = "existing.txt";
  QFile file(m_tmp_dir + file_name);
  QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
  QTextStream out_stream(&file);
  QString initial_text = "Initial text";
  out_stream << initial_text;
  file.close();
  int orig_size = file.size();

  // Open the text file
  m_transcribe->openTextFile(m_tmp_dir + file_name);

  // Compare the text in the editor window
  QObject* text_area = m_app_window->findChild<QObject*>("text_area");
  QVariant text_area_cont = QQmlProperty::read(text_area, "text");
  QCOMPARE(text_area_cont.toString(), initial_text);

  // Check if the file is kept intact
  QVERIFY(file.exists());
  QCOMPARE(file.size(), orig_size);
  QCOMPARE(m_transcribe->getTextFileName(), file_name);
}

void TranscribeTest::unwritableTextFile() {
  QString file_name = "readonly.txt";
  QFile file(m_tmp_dir + file_name);
  QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
  file.close();
  QVERIFY(file.setPermissions(QFileDevice::ReadOwner));

  m_transcribe->openTextFile(file_name);

  QObject* text_area = m_app_window->findChild<QObject*>("text_area");
  QVariant editor_pos(0);
  QVariant editor_text("Hello world!");
  QMetaObject::invokeMethod(text_area, "insert", QGenericReturnArgument(),
                            Q_ARG(QVariant, editor_pos),
                            Q_ARG(QVariant, editor_text));

  QVERIFY(m_transcribe->isTextDirty());
  QTimer::singleShot(500, this, SLOT(dismissMessageBox()));
  m_transcribe->saveText();

  QVERIFY(m_is_msg_box_dismissed);
  QVERIFY(m_transcribe->isTextDirty());
}

void TranscribeTest::saveTextChanges() {
  QString file_name = "new.txt";

  // Open an empty text file
  m_transcribe->openTextFile(m_tmp_dir + file_name);
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

  QFile file(m_tmp_dir + file_name);
  file.open(QIODevice::ReadOnly | QIODevice::Text);
  QTextStream in_stream(&file);
  QString file_text(in_stream.readAll());
  QCOMPARE(file_text, hello_world);
}

void TranscribeTest::unloadTextFileOnAudioOpening() {
  QString file_name = "new.txt";
  m_transcribe->openTextFile(m_tmp_dir + file_name);
  QCOMPARE(m_transcribe->getTextFileName(), file_name);

  QString silence_file = QString(SRCDIR);
  silence_file += "files/silence.wav";
  m_transcribe->openAudioFile(silence_file);

  QCOMPARE(m_transcribe->getTextFileName(), QString("No transcript file loaded"));
}
