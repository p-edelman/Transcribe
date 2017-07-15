#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QAbstractListModel>

#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QString>
#include <QStringList>

#include <vector>

struct FilePaths {
  QString text;
  QString audio;
};

/** Save the history of opened files, in the order of opening (the latest file
 *  is presented first.
 *  History items consist of a path to a text file and a path to the associated
 *  audio file. Each text file can occur just once on the list, while audio
 *  files can theoretically be associated with multiple text files. */
class HistoryModel : public QAbstractListModel {
  Q_OBJECT

public:
  /** Data roles for querying the appropriate paths with the data() method. */
  enum HistoryRoles {
    AudioFileRole = Qt::UserRole + 1,
    TextFileRole
  };

  /** Instantiate the model and restore the history from the config file. */
  explicit HistoryModel(QObject* parent = 0);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;

public:
  /** Add an item to the list. If this changes the list, the changes are
   *  written to the config file.
   *  @param text_file_path
   *  @param audio_file_path */
  void add(QString text_file_path, QString audio_file_path);

private:
  /** Save the history to the config file. */
  void saveHistory();

  /** The actual history. The items are saved in order using a std::vector. */
  std::vector<FilePaths> m_paths;

  /** The name of the group in the config file. */
  const QString CFG_GROUP = "history";
};

#endif // HISTORYMODEL_H
