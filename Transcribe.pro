TEMPLATE = app

QT += qml quick multimedia
CONFIG += c++11

SOURCES += main.cpp \
    transcribe.cpp \
    audioplayer.cpp \
    keycatcher.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    transcribe.h \
    audioplayer.h \
    keycatcher.h

DISTFILES += \
    README.md
