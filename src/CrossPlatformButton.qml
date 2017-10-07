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

  width: (icon.width > 1 && icon.height > 1) ? height: implicitWidth

  // We can't use iconSource because it doesn't work well with image providers
  // (at least on Linux), so we simply draw an image
  Image {
    id: icon
    source: "image://translatedicon/" + icon_id + (enabled ? "" : "/inactive")
    sourceSize.width:  parent.height * 0.75
    sourceSize.height: parent.height * 0.75
    anchors.centerIn: parent
  }

  text: (icon.width == 1 && icon.height == 1) ? fallback_text: ""
}
