#include "materialiconprovider.h"

MaterialIconProvider::MaterialIconProvider() : QQuickImageProvider(Pixmap) {
  QFontDatabase::addApplicationFont(":/fonts/MaterialIcons.ttf");
}

QPixmap MaterialIconProvider::requestPixmap(const QString& identifier, QSize* size,
                                            const QSize& requested_size) {

  if (size) {
    *size = QSize(DEFAULT_SIZE, DEFAULT_SIZE);
  }

  // Create the pixmap
  QPixmap pixmap(requested_size.width()  > 0 ? requested_size.width()  : DEFAULT_SIZE,
                 requested_size.height() > 0 ? requested_size.height() : DEFAULT_SIZE);

  // Analyze the request
  QStringList parts = identifier.split('/');
  QString id = parts[0];
  bool is_active = true;
  if (parts.count() == 2) {
    if (parts[1] == "inactive") {
      is_active = false;
    }
  }

  //pixmap.fill(QColor(Qt::transparent));
  pixmap.fill(QColor(Qt::red));

  if (id == "open") {
    fromMaterialFont(QChar(0xE145), is_active, &pixmap);
  } else if (id == "save") {
    fromMaterialFont(QChar(0xE161), is_active, &pixmap);
  } else if (id == "send") {
    fromMaterialFont(QChar(0xE163), is_active, &pixmap);
  } else if (id == "media-playback-start") {
    fromMaterialFont(QChar(0xE037), is_active, &pixmap);
  } else if (id == "media-seek-backward") {
    fromMaterialFont(QChar(0xE020), is_active, &pixmap);
  } else if (id == "media-seek-forward") {
    fromMaterialFont(QChar(0xE01F), is_active, &pixmap);
  }

  return pixmap;
}

void MaterialIconProvider::fromMaterialFont(QChar code, bool is_active,
                                            QPixmap* pixmap) {
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
}
