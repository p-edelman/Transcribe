#include "historymodel.h"

HistoryModel::HistoryModel(QObject* parent) : QAbstractListModel(parent) {
  QSettings settings;
  settings.beginGroup(CFG_GROUP);

  for (QString number : settings.allKeys()) {
    bool is_uint;
    number.toUInt(&is_uint); // Determine if key is an unsigned int

    QStringList parts = settings.value(number).toStringList();

    if (is_uint && parts.size() == 3) {
      HistoryEntry entry;
      entry.text_file  = parts[0];
      entry.audio_file = parts[1];
      entry.audio_pos  = parts[2].toInt();

      QFileInfo text_file(entry.text_file);
      if (text_file.exists()) {
        m_items.push_back(entry);
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

  return m_items.size();
}

QVariant HistoryModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (role == Qt::DisplayRole) {
    QFileInfo info(m_items[index.row()].text_file);
    return QVariant(info.baseName());
  } else if (role == AudioFileRole) {
    return QVariant(m_items[index.row()].audio_file);
  } else if (role == TextFileRole) {
    return QVariant(m_items[index.row()].text_file);
  } else if (role == AudioPostionRole) {
    return QVariant(m_items[index.row()].audio_pos);
  }

  return QVariant();
}

void HistoryModel::add(QString text_file_path,
                       QString audio_file_path,
                       qint64 audio_pos) {
  // Check if text file already exists in history
  for (int i = 0; i < (int)m_items.size(); i++) {
    if (m_items[i].text_file == text_file_path) {
      beginRemoveRows(QModelIndex(), i, i);
      m_items.erase(m_items.begin() + i);
      endRemoveRows();
      i--;
    }
  }

  // Construct new HistoryEntry object
  HistoryEntry entry;
  entry.text_file  = text_file_path;
  entry.audio_file = audio_file_path;
  entry.audio_pos  = audio_pos;

  // Insert it into the model
  beginInsertRows(QModelIndex(), 0, 0);
  m_items.insert(m_items.begin(), entry);
  endInsertRows();

  // Finally, save the data back to the config file
  saveHistory();
}

bool HistoryModel::textFileForAudio(const QString& audio_path,
                                    QString& text_path) {
  for (HistoryEntry const& entry : m_items) {
    if (entry.audio_file == audio_path) {
      text_path.clear();
      text_path.append(entry.text_file);
      return true;
    }
  }

  return false;
}

void HistoryModel::del(HistoryRoles by, const QFile* file) {
  // Get the absolute path for the given file
  QString file_path = QFileInfo(*file).absoluteFilePath();

  // Iterate over the history items and remove all matches
  std::vector<HistoryEntry>::iterator it = m_items.begin();
  while(it != m_items.end()) {
    if (((by == HistoryRoles::AudioFileRole) && (it->audio_file == file_path)) ||
        ((by == HistoryRoles::TextFileRole)  && (it->text_file  == file_path))) {
      beginRemoveRows(QModelIndex(), it - m_items.begin(), it - m_items.begin());
      it = m_items.erase(it);
      endRemoveRows();
    } else {
      it++;
    }
  }

  // Save the changes
  saveHistory();
}

void HistoryModel::saveHistory() {
  QSettings settings;
  settings.beginGroup(CFG_GROUP);

  for (QString key : settings.allKeys()) {
    settings.remove(key);
  }

  int i = 0;
  for (HistoryEntry entry: m_items) {
    QStringList parts;
    parts.append(entry.text_file);
    parts.append(entry.audio_file);
    parts.append(QString::number(entry.audio_pos));
    settings.setValue(QString::number(i), parts);
    i++;
  }
  settings.sync();
}
