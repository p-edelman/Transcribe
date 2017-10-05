#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QQuickImageProvider>

#include <QChar>
#include <QFontDatabase>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QString>

#include <QDebug>

/** Class to provide Google's Material design icon equivalents for some
 *  'icon names' (as used by the iconName property in QML).
 *  It uses the Material Icons font and can provide both 'active' and
 *  'inactive' versions; for the latter the string '/inactive' needs to be
 *  appended. */
class MaterialIconProvider : public QQuickImageProvider {

public:
  MaterialIconProvider();

  QPixmap requestPixmap(const QString& identifier, QSize* size,
                        const QSize& requested_size);

private:
  /** Draw the specified char code onto the provided pixmap, in active or
   *  inactive form. */
  void fromMaterialFont(QChar code, bool is_active, QPixmap* pixmap);

  // TODO
  const int DEFAULT_SIZE = 50;
};

#endif // ICONPROVIDER_H
