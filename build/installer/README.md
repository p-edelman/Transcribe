This configuration allows for building a Windows intaller using the Qt Installer Framework.

Building an installer requires some manual steps. Here they are:

* Make sure the Qt Installer Framework is installed
* Open a terminal
* Make sure the following paths are added to the Windows `%PATH%` variable:
  * path to the Qt command line tools (e.g. `c:\Qt\5.9.1\mingw53_32\bin\`)
  * path to `gcc.exe` installed along with Qt (e.g. `c:\Qt\Tools\mingw530_32\bin\`)
  * path to the Qt Installer Framework command line tools (e.g. `c:\Qt\Tools\QtInstallerFramework\2.0\bin`)
* Build a Release version of the application
* In the terminal, navigate to its build folder (e.g. `build-Transcribe-Desktop_Qt...-Release`). In it, there should be a folder called `release`.
* Make a new folder next to it called `transcribe`, and move the file `release\Transcribe.exe` to this new folder.
* Now we're going to collect all the necessary distribution files into that folder with the following command: `windeployqt --qmldir ..\src transcribe\Transcribe.exe`
* If everything is as it should be, now we can execute the `Transcribe.exe` app in this folder directly. On to the packaging!
* Pack the `transcribe` folder into a `.7z` file with `archivegen.exe transcribe.7z transcribe`
* Now we move on to the `installer` dir in the project root
* Copy the just generated `transcribe.7z` file into `installer\packages\org.mrpi.transcribe\data`
* Now the installer can be built with `binarycreator.exe -c config\config.xml -p packages TranscribeInstaller.exe`
