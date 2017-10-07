# Building an appimage

1. Copy the executable to this dir
2. Add the path to qmake that is used to build the executable to $PATH (`export PATH=/path/to/Qtdir/bin/:$PATH`)
3. From the directory below, run [linuxdeployqt](https://github.com/probonopd/linuxdeployqt): `linuxdeployqt appimage/Transcribe -appimage -qmldir=../src/`
