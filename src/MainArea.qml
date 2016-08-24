import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import AudioPlayer 1.0

/** The main part of the app, with audio controls and a text area to type in.
    This component purely takes care of the UI side of things; it emits signals
    when the user interacts with it, and has methods to change the appearance,
    but it should be coupled to an audio player component to respond to these
    signals and to call these methods. */
Item {
  /** Emitted when the user changes the value of the slider.
      @param seconds the time of the slider in seconds */
  signal valueChanged(int seconds)

  /** Emitted when the playing status is changed by the user.
      @param is_playing indicates whether the user wants the audio to play. */
  signal playingStateChanged(bool is_playing)

  id: main_area

  /** Slider and visual controls to control the audio. */
  Item {
    id: media_controls

    anchors.top:   parent.top
    anchors.left:  parent.left
    anchors.right: parent.right

    implicitHeight: slider.height + play_pause_btn.height

    Text {
      id:      curr_time
      enabled: player.duration > 0 ? true : false

      text: {
        if (slider.pressed) {
          formatSeconds(slider.value)
        } else {
          formatSeconds(player.position)
        }
      }
      anchors.left: parent.left
      anchors.top:  parent.top
    }

    Slider {
      id:      slider
      enabled: player.duration > 0 ? true : false

      orientation:              Qt.Horizontal
      updateValueWhileDragging: true
      stepSize:                 1.0
      maximumValue:             player.duration

      Connections {
        target: player
        onPositionChanged: {
          if (!slider.pressed) slider.value = player.position
        }
      }

      anchors.right: end_time.left
      anchors.top:   parent.top
      anchors.left:  curr_time.right

      /* Sends a signal that the user changed the value on the slider. */
      onPressedChanged: {
        // Note that we can't use onValueChanged, because that signal is also sent
        // after programmatic changes (thus to slider updates from the audio
        // player). So we need to check if the slider is actually pressed and
        // released.
        if (!pressed) main_area.valueChanged(value)
      }
    }

    Text {
      id:      end_time
      text:    formatSeconds(player.duration)
      enabled: player.duration > 0 ? true : false

      anchors.right: parent.right
      anchors.top:   parent.top
    }

    Button {
      id:        play_pause_btn
      checkable: true
      enabled: player.duration > 0 ? true : false
      text:    player.state === PlayerState.PAUSED ? qsTr("Play") : qsTr("Pause")

      anchors.horizontalCenter: slider.horizontalCenter
      anchors.top:              slider.bottom

      // We cannot directly bind the checked state here because it can get
      // overruled by the user, and then the binding is lost. So we need to
      // explicitly respond to the signal here,
      Connections {
        target: player
        onStateChanged: {
          if (player.state === PlayerState.PAUSED) {
            play_pause_btn.checked = false
          } else {
            play_pause_btn.checked = true
          }
        }
      }

      onClicked: {
        main_area.playingStateChanged(checked)
      }
    }

    CheckBox {
      id:   waiting_check;
      text: qsTr("Waiting")

      enabled: false
      checked: player.state === PlayerState.WAITING ? true : false

      anchors.left:           play_pause_btn.right
      anchors.verticalCenter: play_pause_btn.verticalCenter
    }
  }

  TextArea {
    id:         text_area
    objectName: "text_area"

    anchors.top:    media_controls.bottom
    anchors.right:  parent.right
    anchors.bottom: parent.bottom
    anchors.left:   parent.left

    // Text input seems to be somewhat borked at the moment on touch based
    // platforms (or at least Android). Touch is registered as a mouse click,
    // so the user can't drag/flick long text but ends up selecting it.
    // To prevent this, we can set the ability to select by mouse to false on
    // touch based platforms. This is of course a crude tool; it's now not
    // possible to use an attached mouse to select text. Worse, it's also
    // not possible to select text the proper touch based way, with a long
    // press, only the keyboard will work.
    selectByMouse: {
      switch (Qt.platform.os) {
      case "android":
      case "blackberry":
      case "ios":
      case "winphone":
        false
        break
      default:
        true
      }
    }

    readOnly:            !main_window.is_editable
    focus:               true
    font.pointSize:      (Qt.platform.os == "android") ? 16 : 12
    textFormat:          Text.PlainText
    horizontalAlignment: Text.AlignLeft
    wrapMode:            TextEdit.WordWrap
    flickableItem {
      flickableDirection: Flickable.VerticalFlick
    }

    // Set the text to dirty status whenever it changes
    onTextChanged: app.is_text_dirty = true

    // On loading, the text gets changed so the status gets set to dirty, even
    // though the user didn't change anything. So we have to reset it after
    // loading.
    Component.onCompleted: app.is_text_dirty = false

    style: TextAreaStyle {
      backgroundColor: {
        if (main_window.is_editable) {
          "white";
        } else {
          "lightgray"
        }
      }
    }
  }

  /** Internal function to convert seconds to h.mm:ss strings. */
  function formatSeconds(seconds) {
    seconds     = parseInt(seconds)
    var hours   = Math.floor(seconds / 3600)
    seconds     = seconds % 3600
    var minutes = Math.floor(seconds / 60)
    seconds     = seconds % 60

    var str = "" + hours + "."
    if (minutes < 10) {
      str += "0"
    }
    str += minutes + ":"
    if (seconds < 10) {
      str += "0"
    }
    str += seconds

    return str
  }
}
