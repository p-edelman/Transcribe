#-------------------------------------------------
#
# Project created by QtCreator 2016-06-11T11:53:25
#
#-------------------------------------------------

QT       += testlib multimedia
QT       -= gui

TARGET = tst_audioplayertest
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app

INCLUDEPATH += ../src
VPATH       += ../src

SOURCES += tst_audioplayertest.cpp \
           audioplayer.cpp

HEADERS += audioplayer.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
