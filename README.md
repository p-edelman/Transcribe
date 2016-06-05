# Transcribe

A tool for people who need to transcribe audio files to text (especially when they are slow typers with sticky fingers).

The app allows you to load an audio file and play it while typing out the text, so you don't have to switch constantly between audio player and text editor. Crucially, the app provides key bindings so that:
* you can type without interfering with the playback of the audio, and conversily
* you can control audio starting, stopping and seeking without interfering with the text editing, and
* you don't accidentaly type something that resets the audio, skips to the next song, etc.

Furthermore, if you're frantically typing to keep up with the audio, the app will pause every few seconds until you're there.

The app is written using het Qt framework runs on Android and Linux. It should also work on Mac an Windows when someone compiles it on those platforms.

## Status

It works, but it's very rough around the edges and lacks quality controls, so there's no released version yet. You're of course free to checkout the repository.

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
