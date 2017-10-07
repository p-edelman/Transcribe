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

  // Make sure our height is an even number, because that makes it so much
  // easier to properly position the icon
  height: (implicitHeight % 2 === 0) ? implicitHeight : implicitHeight + 1

  // Set width equal to the height if we have an icon
  width: (icon.width > 1 && icon.height > 1) ? height: implicitWidth

  // We can't use iconSource because it doesn't work well with image providers
  // (at least on Linux), so we simply draw an image
  Image {
    id: icon
    source: "image://translatedicon/" + icon_id + (enabled ? "" : "/inactive")

    // Again, request an icon with an even number of pixels, because that makes
    // it easier to position it properly
    function calcSize() {
      var size = Math.round(parent.height * 0.75)
      if (size % 2 !== 0) size += 1
      return size
    }

    sourceSize.width:  calcSize()
    sourceSize.height: calcSize()
    anchors.centerIn:  parent
  }

  text: (icon.width == 1 && icon.height == 1) ? fallback_text: ""
}
