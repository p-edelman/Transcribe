#ifndef TRANSCRIBE_H
#define TRANSCRIBE_H

#include <QObject>

/** The main application class. */
class Transcribe : public QObject
{
  Q_OBJECT
public:
  explicit Transcribe(QObject *parent = 0);

signals:

public slots:
};

#endif // TRANSCRIBE_H
