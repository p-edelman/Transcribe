#ifndef ICONTRANSLATIONMATRIX_H
#define ICONTRANSLATIONMATRIX_H

#include <QQuickImageProvider>

#include <QChar>
#include <QFontDatabase>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QString>

#include <QDebug>

/** Provide icons in a (kind of) cross platform way.
 *  Icons can be requested using the Freedesktop naming conventions. On Linux
 *  the theme icon is provided, on Android the corresponding one from Google's
 *  Material design set. The Material icons are also used as fallback. If no
 *  icon can be found, a pixmap with size (1, 1) is returned.
 *  Both 'active' and 'inactive' versions can be queried; for the latter the
 *  string '/inactive' needs to be appended to the url. */
class IconTranslationMatrix : public QQuickImageProvider {

public:
  IconTranslationMatrix();

  QPixmap requestPixmap(const QString& identifier, QSize* size,
                        const QSize& requested_size);

private:
  /** Draw the specified icon from the theme onto the provided pixmap, adapted
   *  to the size of said pixmap, in active or inactive form.
   *  If the icon couldn't be obtained from the theme, this method returns
   *  false. */
  bool fromTheme(QString name, bool is_active, QPixmap* pixmap);

  /** Draw the specified char code onto the provided pixmap, in active or
   *  inactive form.
   *  If the icon isn't available, this method returns false. */
  bool fromMaterialFont(QChar code, bool is_active, QPixmap* pixmap);

  // TODO
  const int DEFAULT_SIZE = 100;
};

#endif // ICONTRANSLATIONMATRIX_H
