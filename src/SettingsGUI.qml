import QtQuick 2.7
import QtQuick.Controls 1.4

import "Constants.js" as Constants

/** A GUI for adjusting settings for the app. */
Item {
  id:         config_window
  objectName: "config_window"

  /** Emitted when the user wants to dismiss the settings dialog. */
  signal settingsDone()

  FontMetrics { // The default font metrics, which we can manipulate
    id: font_metrics
  }

  Text {
    id: timeout_header_text

    text:               qsTr("Timeouts for typing")
    anchors.left:       parent.left
    anchors.leftMargin: Constants.margin
    anchors.top:        parent.top
    anchors.topMargin:  Constants.margin
  }

  Text {
    id: wait_text

    text:               qsTr("Pause audio when typing uninterrupted for:")
    wrapMode:           Text.WordWrap
    anchors.top:        timeout_header_text.bottom
    anchors.left:       parent.left
    anchors.leftMargin: Constants.margin
    font.pointSize:     font_metrics.font.pointSize * 0.9
  }

  Slider {
    id: wait_timeout_slider

    anchors.rightMargin: 0
    anchors.leftMargin:  Constants.margin
    anchors.right:       wait_timeout_value.left
    anchors.top:         wait_text.bottom
    anchors.left:        parent.left
    orientation:         Qt.Horizontal
    minimumValue: {
      // Make sure the wait timeout is always smaller than the type timeout
      if (type_timeout_slider.value < typingtimelord.wait_timeout_min) {
        typingtimelord.wait_timeout_min
      } else {
        type_timeout_slider.value + 100
      }
    }
    maximumValue: typingtimelord.wait_timeout_max;
    stepSize:     100
  }

  Text {
    id: wait_timeout_value

    text: (wait_timeout_slider.value / 1000).toLocaleString(Qt.locale(), "f", 1) + " s"

    textFormat:             Text.PlainText
    font.pointSize:         font_metrics.font.pointSize * 0.9
    horizontalAlignment:    Text.AlignLeft
    anchors.verticalCenter: wait_timeout_slider.verticalCenter
    anchors.right:          parent.right
    anchors.rightMargin:    Constants.margin
  }

  Text {
    id: type_text

    text: qsTr("Consider typing has stopped after:")

    wrapMode:           Text.WordWrap
    anchors.left:       parent.left
    anchors.leftMargin: Constants.margin
    anchors.top:        wait_timeout_slider.bottom
    font.pointSize:     font_metrics.font.pointSize * 0.9
  }

  Slider {
    id: type_timeout_slider

    tickmarksEnabled:    false
    anchors.right:       type_timeout_value.left
    anchors.rightMargin: 0
    anchors.leftMargin:  Constants.margin
    anchors.left:        parent.left
    anchors.top:         type_text.bottom
    orientation: Qt.Horizontal
    minimumValue: typingtimelord.type_timeout_min;
    maximumValue: typingtimelord.type_timeout_max;
    stepSize: 100
  }

  Text {
    id: type_timeout_value

    text: (type_timeout_slider.value / 1000).toLocaleString(Qt.locale(), "f", 1) + " s"

    textFormat:             Text.PlainText
    font.pointSize:         font_metrics.font.pointSize * 0.9
    anchors.verticalCenter: type_timeout_slider.verticalCenter
    anchors.right:          parent.right
    anchors.rightMargin:    Constants.margin
  }

  // The button to dismiss the settings GUI
  Button {
    anchors.right:   parent.right
    anchors.bottom:  parent.bottom
    anchors.margins: Constants.margin
    text:            qsTr("Done")
    onClicked: {
      typingtimelord.wait_timeout = wait_timeout_slider.value
      typingtimelord.type_timeout = type_timeout_slider.value
      config_window.settingsDone()
    }
  }

  // 'Initialize' the settings GUI when it becomes visible.
  onVisibleChanged: {
    if (visible) {
      wait_timeout_slider.value = typingtimelord.wait_timeout
      type_timeout_slider.value = typingtimelord.type_timeout
    }
  }
}
