# Building an appimage

1. Compile
2. Copy everything from this dir to the build dir
3. Rename the build dir to 'transcribe'
4. Add the path to qmake that is used to build the executable to $PATH (`export PATH=/path/to/Qtdir/bin/:$PATH`)
5. From the root directory, run [linuxdeployqt](https://github.com/probonopd/linuxdeployqt): `linuxdeployqt transcribe/Transcribe -appimage -qmldir=src/`
