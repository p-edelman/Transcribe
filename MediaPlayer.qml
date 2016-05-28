import QtQuick 2.0
import QtQuick.Controls 1.4

/** Provides a slider and visual controls to control the audio.
    This component purely takes care of the UI side of things; it emits signals
    when the user interacts with it, and has methods to change the appearance,
    but it should be coupled to an audio player component to respond to these
    signals and to call these methods. */
Item {
  /** Emitted when the user changes the value of the slider. */
  signal valueChanged(int seconds)

  id: media_player

  anchors.top:   parent.top
  anchors.left:  parent.left
  anchors.right: parent.right

  Text {
    id:    start_time
    text: "0.00:00"

    anchors.left: parent.left
    anchors.top:  parent.top
  }

  Slider {
    id: slider

    orientation:              Qt.Horizontal
    updateValueWhileDragging: false
    stepSize:                 1.0

    anchors.right: end_time.left
    anchors.top:   parent.top
    anchors.left:  start_time.right

    onValueChanged: {
      media_player.valueChanged(value)
    }
  }

  Text {
    id:    end_time
    text: "0.00:00"

    anchors.right: parent.right
    anchors.top:   parent.top
  }

  /** Set the duration for the media player to the specified amount of seconds */
  function setDuration(seconds) {
    end_time.text       = formatSeconds(seconds)
    slider.maximumValue = seconds
    console.log("Setting duration to " + seconds)
  }

  /** Set the current position in the audio stream to the specified amount
      of seconds. */
  function setPosition(seconds) {
    slider.value = seconds
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
