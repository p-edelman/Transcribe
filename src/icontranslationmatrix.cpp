#include "icontranslationmatrix.h"

IconTranslationMatrix::IconTranslationMatrix() : QQuickImageProvider(Pixmap) {
  QFontDatabase::addApplicationFont(":/fonts/MaterialIcons.ttf");
}

QPixmap IconTranslationMatrix::requestPixmap(const QString& identifier,
                                            QSize* size,
                                            const QSize& requested_size) {

  bool has_icon = false;

  // Create the pixmap
  QPixmap pixmap(requested_size.width()  > 0 ? requested_size.width()  : DEFAULT_SIZE,
                 requested_size.height() > 0 ? requested_size.height() : DEFAULT_SIZE);
  pixmap.fill(QColor(Qt::transparent));

  // Analyze the request
  QStringList parts = identifier.split('/');
  QString id = parts[0];
  bool is_active = true;
  if (parts.count() == 2) {
    if (parts[1] == "inactive") {
      is_active = false;
    }
  }

#ifdef Q_OS_ANDROID
  has_icon = fromMaterialFont(id, is_active, &pixmap);
#else
  has_icon = fromTheme(id, is_active, &pixmap);
  if (!has_icon) {
    // Fallback to Material icon
    has_icon = fromMaterialFont(id, is_active, &pixmap);
  }
#endif

  // If we didn't have an icon, return a size of (1, 1)
  if (!has_icon) {
    pixmap = QPixmap(1, 1);
    if (size) *size = QSize(1, 1);
  } else if (size) {
    *size = QSize(DEFAULT_SIZE, DEFAULT_SIZE);
  }

  return pixmap;
}

bool IconTranslationMatrix::fromTheme(QString name, bool is_active,
                                     QPixmap *pixmap) {
  QIcon icon = QIcon::fromTheme(name, QIcon());
  if (icon.isNull()) {
    return false;
  }

  // Draw the icon on the pixmap. The actual icon size is probably smaller than
  // the size we need, so we need to center it.
  QPainter painter(pixmap);
  QIcon::Mode mode = (is_active) ? QIcon::Normal : QIcon::Disabled;
  QSize actual = icon.actualSize(pixmap->size(), mode);
  QSize req    = pixmap->size();
  painter.drawPixmap(req.width()  / 2 - actual.width()  / 2,
                     req.height() / 2 - actual.height() / 2,
                     icon.pixmap(req, mode));
  painter.end();

  return true;
}

QChar IconTranslationMatrix::getCharForIconId(QString id) {
  QChar code(0);
  if (id == "open") {
    code = QChar(0xE145);
  } else if (id == "save") {
    code = QChar(0xE161);
  } else if (id == "send") {
    code = QChar(0xE163);
  } else if (id == "media-playback-start") {
    code = QChar(0xE037);
  } else if (id == "media-seek-backward") {
    code = QChar(0xE020);
  } else if (id == "media-seek-forward") {
    code = QChar(0xE01F);
  }

  return code;
}

bool IconTranslationMatrix::fromMaterialFont(QString id, bool is_active,
                                             QPixmap* pixmap) {
  QChar code = getCharForIconId(id);
  if (code == QChar(0)) return false;

  QPainter painter(pixmap);
  QFont font("Material Icons");
  font.setPixelSize(pixmap->height());
  painter.setFont(font);

  QPen pen;
  if (is_active) {
    pen.setColor(QColor(Qt::black)); // TODO: Use system colors
  } else {
    pen.setColor(QColor(Qt::gray));
  }
  painter.setPen(pen);

  painter.drawText(0, 0, pixmap->width(), pixmap->height(), Qt::AlignHCenter, code);

  return true;
}
