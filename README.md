# Transcribe

A tool for people who need to transcribe audio recordings to text (more specifically, people that are slow typists and have sticky fingers ;)

The app is quite simple; it allows you to load an audio file and play it while typing out the text, so you don't have to switch constantly between audio player and text editor. Crucially, the app:
* Provides key bindings so that:
	* you can type without interfering with the playback of the audio, and conversely
	* you can control the audio (starting, stopping and seeking) without interfering with the text editing, and
	* you don't accidentally type something that resets the audio, skips to the next song, etc.
* Pauses the audio playback whenever you're frantically typing to keep up

The app is written using het Qt framework and is tested on Android and Linux. It should be possible to compile it on Windows and Mac without much trouble.

## Status

The first stable release, codenamed 'Amy' is totally usable, though somewhat rough around the edges. You can find the Android version [in the Play Store](https://play.google.com/store/apps/details?id=org.mrpi.transcribe), or use the tag 'amy' to check out the Git repository.

## The very short manual

From the menu, you can open an audio file. Immediately after that, you will be asked to open a text file for saving the transcript text. If you pick an existing file, the text will be loaded in the editor window.

You can control the audio playback with the following keys:
* Start/stop: <CTRL>+<Space>, or the multimedia keys on you keyboard if you have them.
* Skip 5 seconds forward and backward: <Alt>+<Left> and <Alt+Right>, or the <Previous> and <Next> multimedia keys.

From then on, you can happily type away along with the audio playback. The audio will play in 5 second intervals and wait if you're still typing. If you pause typing for 1 second, the audio playback continues (or isn't paused).

*You're completely responsable for saving the text file by pressing <CTRL>+s once in a while or using the menu.*

And that's all there is to it!

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.en.html)
