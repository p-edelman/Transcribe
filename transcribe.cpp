#include "transcribe.h"

Transcribe::Transcribe(int& argc, char** argv) : QGuiApplication(argc, argv) {
  m_player = new QMediaPlayer;
}
