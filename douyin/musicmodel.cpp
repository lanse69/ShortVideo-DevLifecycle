#include "musicmodel.h"
#include <QDebug>
#include <QTime>

MusicModel::MusicModel(QObject *parent)
    : QAbstractListModel(parent) {
    initializeLocalMusic();
}

int MusicModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_musicList.size();
}

QVariant MusicModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_musicList.size())
        return QVariant();

    const MusicItem &item = m_musicList.at(index.row());

    switch (role) {
    case IdRole:
        return item.id;
    case TitleRole:
        return item.title;
    case ArtistRole:
        return item.artist;
    case AlbumRole:
        return item.album;
    case CoverUrlRole:
        return item.coverUrl;
    case AudioUrlRole:
        return item.audioUrl;
    case DurationRole:
        return item.duration;
    case IsFavoriteRole:
        return item.isFavorite;
    case IsPlayingRole:
        return item.isPlaying;
    case PlayCountRole:
        return item.playCount;
    case FormattedDurationRole: {
        int minutes = item.duration / 60;
        int seconds = item.duration % 60;
        return QString("%1:%2").arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MusicModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "musicId";
    roles[TitleRole] = "title";
    roles[ArtistRole] = "artist";
    roles[AlbumRole] = "album";
    roles[CoverUrlRole] = "coverUrl";
    roles[AudioUrlRole] = "audioUrl";
    roles[DurationRole] = "duration";
    roles[IsFavoriteRole] = "isFavorite";
    roles[IsPlayingRole] = "isPlaying";
    roles[PlayCountRole] = "playCount";
    roles[FormattedDurationRole] = "formattedDuration";
    return roles;
}

void MusicModel::loadLocalMusic() {
    // 清空现有数据
    if (!m_musicList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_musicList.size() - 1);
        m_musicList.clear();
        endRemoveRows();
    }

    // 重新加载本地音乐
    initializeLocalMusic();

    if (!m_musicList.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, m_musicList.size() - 1);
        // 数据已经在initializeLocalMusic中添加到m_musicList
        endInsertRows();
    }

    emit dataChanged(index(0), index(m_musicList.size() - 1));
}

void MusicModel::addMusic(const QString &title, const QString &artist,
                          const QString &audioUrl, int duration) {
    beginInsertRows(QModelIndex(), m_musicList.size(), m_musicList.size());

    MusicItem item;
    item.id = QString("music_%1_%2").arg(m_musicList.size()).arg(QTime::currentTime().msec());
    item.title = title;
    item.artist = artist;
    item.audioUrl = audioUrl;
    item.duration = duration;
    item.album = "用户上传";
    item.coverUrl = "qrc:/images/default_music_cover.png";

    m_musicList.append(item);
    endInsertRows();
}

void MusicModel::removeMusic(int index) {
    if (index < 0 || index >= m_musicList.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_musicList.removeAt(index);
    endRemoveRows();
}

void MusicModel::toggleFavorite(int index) {
    if (index < 0 || index >= m_musicList.size())
        return;

    m_musicList[index].isFavorite = !m_musicList[index].isFavorite;
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {IsFavoriteRole});
}

void MusicModel::setPlaying(int index) {
    // 先重置所有播放状态
    for (int i = 0; i < m_musicList.size(); ++i) {
        if (m_musicList[i].isPlaying && i != index) {
            m_musicList[i].isPlaying = false;
            QModelIndex modelIndex = createIndex(i, 0);
            emit dataChanged(modelIndex, modelIndex, {IsPlayingRole});
        }
    }

    // 设置新的播放状态
    if (index >= 0 && index < m_musicList.size()) {
        m_musicList[index].isPlaying = true;
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {IsPlayingRole});
    }
}

void MusicModel::incrementPlayCount(int index) {
    if (index < 0 || index >= m_musicList.size())
        return;

    m_musicList[index].playCount++;
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {PlayCountRole});
}

QVariantMap MusicModel::getMusicInfo(int index) const {
    QVariantMap info;
    if (index < 0 || index >= m_musicList.size())
        return info;

    const MusicItem &item = m_musicList.at(index);
    info["id"] = item.id;
    info["title"] = item.title;
    info["artist"] = item.artist;
    info["album"] = item.album;
    info["coverUrl"] = item.coverUrl;
    info["audioUrl"] = item.audioUrl.toString();
    info["duration"] = item.duration;
    info["isFavorite"] = item.isFavorite;
    info["isPlaying"] = item.isPlaying;
    info["playCount"] = item.playCount;

    return info;
}

QString MusicModel::getAudioUrl(int index) const {
    if (index < 0 || index >= m_musicList.size())
        return QString();

    return m_musicList.at(index).audioUrl.toString();
}

QVariantList MusicModel::searchMusic(const QString &keyword) {
    QVariantList result;
    QString lowerKeyword = keyword.toLower();

    for (int i = 0; i < m_musicList.size(); ++i) {
        const MusicItem &item = m_musicList.at(i);
        if (item.title.toLower().contains(lowerKeyword) ||
            item.artist.toLower().contains(lowerKeyword) ||
            item.album.toLower().contains(lowerKeyword)) {
            QVariantMap musicInfo = getMusicInfo(i);
            musicInfo["index"] = i;
            result.append(musicInfo);
        }
    }

    return result;
}

QVariantList MusicModel::getFavorites() {
    QVariantList result;

    for (int i = 0; i < m_musicList.size(); ++i) {
        if (m_musicList.at(i).isFavorite) {
            QVariantMap musicInfo = getMusicInfo(i);
            musicInfo["index"] = i;
            result.append(musicInfo);
        }
    }

    return result;
}

void MusicModel::initializeLocalMusic() {
    // 使用本地qrc资源初始化音乐数据
    // 这里模拟从服务端获取的数据

    MusicItem music1;
    music1.id = "music_local_1";
    music1.title = "落了白";
    music1.artist = "蒋雪儿";
    music1.album = "落了白";
    music1.coverUrl = "qrc:/images/music_cover_1.png";
    music1.audioUrl = QUrl("qrc:/vm/VideosAndMusics/Musics/落了白.mp3");
    music1.duration = 218; // 3分38秒
    music1.isFavorite = true;

    MusicItem music2;
    music2.id = "music_local_2";
    music2.title = "弱水三千";
    music2.artist = "石头 & 张晓棠";
    music2.album = "古风精选";
    music2.coverUrl = "qrc:/images/music_cover_2.png";
    music2.audioUrl = QUrl("qrc:/vm/VideosAndMusics/Musics/弱水三千.mp3");
    music2.duration = 245; // 4分5秒
    music2.isFavorite = false;

    MusicItem music3;
    music3.id = "music_local_3";
    music3.title = "莫问归期";
    music3.artist = "蒋雪儿";
    music3.album = "莫问归期";
    music3.coverUrl = "qrc:/images/music_cover_3.png";
    music3.audioUrl = QUrl("qrc:/vm/VideosAndMusics/Musics/莫问归期.mp3");
    music3.duration = 235; // 3分55秒
    music3.isFavorite = true;

    m_musicList << music1 << music2 << music3;
}
