import QtQuick 2.0
import QtQuick.Controls 1.4

import AudioPlayer 1.0

/** Provides a slider and visual controls to control the audio.
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

  id: media_player

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
      if (!pressed) media_player.valueChanged(value)
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
      media_player.playingStateChanged(checked)
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
