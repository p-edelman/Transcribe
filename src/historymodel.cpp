#include "historymodel.h"

HistoryModel::HistoryModel(QObject* parent) : QAbstractListModel(parent) {
  QSettings settings;
  settings.beginGroup(CFG_GROUP);

  for (QString number : settings.allKeys()) {
    bool is_uint;
    number.toUInt(&is_uint); // Determine if key is an unsigned int

    QStringList file_list = settings.value(number).toStringList();

    if (is_uint && file_list.size() == 2) {
      FilePaths paths;
      paths.text  = file_list[0];
      paths.audio = file_list[1];

      QFileInfo text_file(paths.text);
      if (text_file.exists()) {
        m_paths.push_back(paths);
      }
    }
  }

  // Save back
  saveHistory();
}

int HistoryModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }

  return m_paths.size();
}

QVariant HistoryModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (role == Qt::DisplayRole) {
    QFileInfo info(m_paths[index.row()].text);
    return QVariant(info.baseName());
  } else if (role == AudioFileRole) {
    return QVariant(m_paths[index.row()].audio);
  } else if (role == TextFileRole) {
    return QVariant(m_paths[index.row()].text);
  }

  return QVariant();
}

void HistoryModel::add(QString text_file_path, QString audio_file_path) {
  // Ignore if the item is already the first item on the list
  if (m_paths.size() > 0) {
    if (m_paths[0].text  == text_file_path &&
        m_paths[0].audio == audio_file_path) {
      return;
    }
  }

  // Check if text file already exists in history
  for (int i = 0; i < (int)m_paths.size(); i++) {
    if (m_paths[i].text == text_file_path) {
      beginRemoveRows(QModelIndex(), i, i);
      m_paths.erase(m_paths.begin() + i);
      endRemoveRows();
      i--;
    }
  }

  // Construct new FilePaths object
  FilePaths paths;
  paths.text  = text_file_path;
  paths.audio = audio_file_path;

  // Insert it into the model
  beginInsertRows(QModelIndex(), 0, 0);
  m_paths.insert(m_paths.begin(), paths);
  endInsertRows();

  // Finally, save the data back to the config file
  saveHistory();
}

void HistoryModel::saveHistory() {
  QSettings settings;
  settings.beginGroup(CFG_GROUP);

  for (QString key : settings.allKeys()) {
    settings.remove(key);
  }

  int i = 0;
  for (FilePaths paths: m_paths) {
    QStringList path_list;
    path_list.append(paths.text);
    path_list.append(paths.audio);
    settings.setValue(QString::number(i), path_list);
    i++;
  }
  settings.sync();
}
