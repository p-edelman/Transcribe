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
           audioplayertest.cpp \
           typingtimelordtest.cpp \
           keycatchertest.cpp \
           transcribetest.cpp \
           ../src/audioplayer.cpp \
           ../src/typingtimelord.cpp \
           ../src/keycatcher.cpp \
           ../src/transcribe.cpp

HEADERS += audioplayertest.h \
           typingtimelordtest.h \
           keycatchertest.h \
           transcribetest.h \
           ../src/audioplayer.h \
           ../src/typingtimelord.h \
           ../src/keycatcher.h \
           ../src/transcribe.h

RESOURCES += ../src/qml.qrc

DEFINES += SRCDIR=\\\"$$PWD/\\\"
