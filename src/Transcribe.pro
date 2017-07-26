TEMPLATE = app

QT += qml quick multimedia widgets
android: QT += androidextras
CONFIG += c++11

SOURCES += main.cpp \
    transcribe.cpp \
    audioplayer.cpp \
    keycatcher.cpp \
    typingtimelord.cpp \
    sonicbooster.cpp \
    audiodecoder.cpp \
    historymodel.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    transcribe.h \
    audioplayer.h \
    keycatcher.h \
    typingtimelord.h \
    sonicbooster.h \
    audiodecoder.h \
    historymodel.h

DISTFILES += \
    README.md \
    ../resources/android/AndroidManifest.xml \
    ../resources/android/gradle/wrapper/gradle-wrapper.jar \
    ../resources/android/gradlew \
    ../resources/android/res/values/libs.xml \
    ../resources/android/build.gradle \
    ../resources/android/gradle/wrapper/gradle-wrapper.properties \
    ../resources/android/src/org/mrpi/transcribe/TranscribeActivity.java \
    ../resources/android/src/org/mrpi/transcribe/TranscriptionSharer.java

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../resources/android

RC_ICONS = ../resources/icon/windows/icon.ico
