# Transcribe

**NOTE**: I'm no longer maintaining this project. I wrote it at a time when it was an "itch to scratch" for me, but since I've switched jobs, I don't have a need for it anymore. That being said, the app probably works as advertised.

A tool for people who need to transcribe audio recordings to text (more specifically, people that are slow typists and have fat fingers ;)

The app is quite simple; it allows you to load an audio file and play it while typing out the text, so you don't have to switch constantly between audio player and text editor. Crucially, the app:

* Pauses the audio playback whenever you're frantically typing to keep up
* Provides key bindings so that:
	* you can type without interfering with the playback of the audio, and conversely
	* you can control the audio (starting, stopping and seeking) without interfering with the text editing, and
	* you don't accidentally type something that resets the audio, skips to the next song, etc.

Additional features:
* Boosting of the audio signal for distant voices without distorting loud voices (too much). This is a better option than turning up the device volume, which might reach dangerous levels when there are loud episodes in the recording.

The app is written using het Qt framework and is tested on Linux, Android (see below) and Windows 10 (see below). It should be possible to compile it on Mac without much trouble.

## Status

The fourth stable release, codenamed 'Dalek' is totally suited for daily usage. Please select your platform:

* [Linux AppImage](https://github.com/p-edelman/Transcribe/releases/download/dalek/transcribe-dalek.AppImage)
* [Windows installer](https://github.com/p-edelman/Transcribe/releases/download/dalek/TranscribeInstaller.exe)
* [Android app ~~from the Play Store~~ as a direct download](https://github.com/p-edelman/Transcribe/releases/download/dalek/Transcribe-dalek.apk)

If you want to build it yourself, you should checkout the tag 'dalek' from Git.

### Android

The experience on Android is less polished than on the desktop, so to speak. This is because it is mainly a desktop app that is ported to Android, which uses quite different concepts at times. It would take significant effort to make it work smoothly on Android.

You need to take into account:

* The app works by opening files from the SD card or internal storage, just like a desktop application and rather unlike a mobile app
* For this you need to enable the 'Storage' permission (the app should ask for this, but if it is somehow not activated: go the the settings of your device, choose the app overview, find Transcribe, go to permissions, enable)
* Even so, the file open dialog is hard to navigate and sometimes doesn't show the files you need the first time. If you can't find your files, you might need to type in `/mnt/sdcard` in the selection and press 'Open' to view the contents of your phone memory
* Audio boosting only works for .wav files

**NOTE**: The app is not available in the Play Store anymore, as it required a lot of maintainance. Google makes you jump through new hoops all the time and there was a lot of vitriol from people who rather spend their time writing negative reviews than reading even the basic description. I simply don't have the time to manage this. The app is still available as a direct download.

### Windows 10

* Audio boosting only works for .wav files

## The very short manual

First: *There is no autosave! You're completely responsable for saving the text file by pressing `<CTRL>`+`s` once in a while or using the menu.* This is a design choice (there's a chance of data loss with autosave, like when when you just cut some text without pasting it or when your cat walks on your keyboard and deletes some stuff).

From the menu, you can open an audio file. Immediately after that, you will be asked to open a text file for saving the transcript text. If you pick an existing file, the text will be loaded in the editor window.

You can control the audio playback with the following keys:

* Start/stop: `<CTRL>`+`<Space>`, or the multimedia keys on you keyboard if you have them.
* Skip 5 seconds forward and backward: `<ALT>`+`<Left>` and `<ALT>`+`<Right>`, or the `<Previous>` and `<Next>` multimedia keys.
* Boost/decrease the volume: `<ALT>`+`<Up>` and `<ALT>`+`<Down>`. This doesn't affect the device volume, just the recording, and you can't get louder than the device volume. *Make sure the device volume stays within comfortable levels at all times!*

From then on, you can happily type away along with the audio playback. The audio will play in intervals and wait for you if you're still typing. If you pause typing, the audio playback continues (or doesn't get paused). The default values are set to 5 seconds for the interval and 1 seconds for the typing pause, but these values can be adjusted in the settings.

And that's all there is to it!

## License

All code is licensed according to the [GPLv3](http://www.gnu.org/licenses/gpl-3.0.en.html).

This app ships with an unmodified copy of the [Material Icons font](https://material.io/icons/), which is distributed according to the terms of the [Apache License Version 2.0](http://www.apache.org/licenses/LICENSE-2.0.txt).
