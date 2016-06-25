TEMPLATE = app

QT += qml quick multimedia widgets
CONFIG += c++11

SOURCES += main.cpp \
    transcribe.cpp \
    audioplayer.cpp \
    keycatcher.cpp \
    typingtimelord.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    transcribe.h \
    audioplayer.h \
    keycatcher.h \
    typingtimelord.h

DISTFILES += \
    README.md \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
