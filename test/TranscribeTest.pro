#-------------------------------------------------
#
# Project created by QtCreator 2016-06-11T11:53:25
#
#-------------------------------------------------

QT       += testlib multimedia
QT       -= gui

TARGET = TranscribeTest
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

#TEMPLATE = app

INCLUDEPATH += ../src
#VPATH       += ../src

SOURCES += main.cpp \
           tst_audioplayertest.cpp \
           tst_typingtimelordtest.cpp \
           ../src/audioplayer.cpp \
           ../src/typingtimelord.cpp

HEADERS += tst_audioplayertest.h \
           tst_typingtimelordtest.h \
           ../src/audioplayer.h \
           ../src/typingtimelord.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
