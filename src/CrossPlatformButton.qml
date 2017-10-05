import QtQuick 2.0
import QtQuick.Controls 1.4

/** A button that kind of adapts to it's platform. This mainly has to do with
    the icons (and the, frankly, crappy way Qt deals with this).
    To select an icon, the icon_id property can be set. On linux this is
    translated to the iconName, while on Android it is used to query the
    MaterialIconProvider class.
    If the fallback_text property is set, it is displayed when there is no icon.
    */
Button {
  property string fallback_text : ""
  property string icon_id       : ""

  iconSource: {
    if ((Qt.platform.os === "android") && (icon_id != "")) {
      var android_icon = "image://materialicon/" + icon_id
      enabled ? android_icon : android_icon + "/inactive"
    } else {
      ""
    }
  }

  iconName: {
    if ((Qt.platform.os === "linux") && (icon_id != "")) {
      icon_id
    } else {
      ""
    }
  }

  Component.onCompleted: {
    if ((iconSource == "") && (iconName == "") && (text == "")) {
      text = fallback_text
    }
  }
}
