#-------------------------------------------------
#
# Project created by QtCreator 2016-06-11T11:53:25
#
#-------------------------------------------------

QT += testlib multimedia widgets qml quick
QT -= gui

TARGET = TranscribeTest
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

INCLUDEPATH += ../src

SOURCES += main.cpp \
           tst_audioplayertest.cpp \
           tst_typingtimelordtest.cpp \
           tst_keycatchertest.cpp \
           ../src/audioplayer.cpp \
           ../src/typingtimelord.cpp \
           ../src/keycatcher.cpp \
           ../src/transcribe.cpp

HEADERS += tst_audioplayertest.h \
           tst_typingtimelordtest.h \
           tst_keycatchertest.h \
           ../src/audioplayer.h \
           ../src/typingtimelord.h \
           ../src/keycatcher.h \
           ../src/transcribe.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
