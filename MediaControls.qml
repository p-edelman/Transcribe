import QtQuick 2.0
import QtQuick.Controls 1.4

/** Provides a slider and visual controls to control the audio.
    This component purely takes care of the UI side of things; it emits signals
    when the user interacts with it, and has methods to change the appearance,
    but it should be coupled to an audio player component to respond to these
    signals and to call these methods. */
Item {
  /** Emitted when the user changes the value of the slider, as time in seconds
    */
  signal valueChanged(int seconds)

  id: media_player

  anchors.top:   parent.top
  anchors.left:  parent.left
  anchors.right: parent.right

  Text {
    id:    curr_time
    text: "0.00:00"

    anchors.left: parent.left
    anchors.top:  parent.top
  }

  Slider {
    id: slider

    orientation:              Qt.Horizontal
    updateValueWhileDragging: true
    stepSize:                 1.0

    anchors.right: end_time.left
    anchors.top:   parent.top
    anchors.left:  curr_time.right

    /* Sends a signal that the user changed the value on the slider.
       Note that we can't use onValueChanged for this, as this also responds
       to programmatic changes (thus to slider updates from the audio player).
     */
    onPressedChanged: {
      if (!pressed) media_player.valueChanged(value)
    }

    /* Display the seek time if the user drags the slider. */
    onValueChanged: {
      if (pressed) curr_time.text = formatSeconds(value)
    }
  }

  Text {
    id:    end_time
    text: "0.00:00"

    anchors.right: parent.right
    anchors.top:   parent.top
  }

  /** Set the duration for the media player to the specified amount of seconds.
    */
  function setDuration(seconds) {
    end_time.text       = formatSeconds(seconds)
    slider.maximumValue = seconds
  }

  /** Set the position of the slider to the specified amount of seconds. */
  function setPosition(seconds) {
    if (!slider.pressed) { // Ignore if the user is dragging the slider
      curr_time.text = formatSeconds(seconds)
      slider.value   = seconds
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
