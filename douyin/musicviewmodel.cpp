#include "musicviewmodel.h"
#include <QDebug>

MusicViewModel::MusicViewModel(QObject *parent)
    : QObject(parent)
    , m_musicModel(new MusicModel(this))
    , m_selectedIndex(-1)
    , m_isPlaying(false)      // 添加这行
    , m_currentPlayingIndex(-1) {  // 添加这行{
}

QAbstractListModel* MusicViewModel::musicList() const {
    return m_musicModel;
}

int MusicViewModel::selectedIndex() const {
    return m_selectedIndex;
}

QString MusicViewModel::selectedMusicTitle() const {
    if (!isValidIndex(m_selectedIndex))
        return QString();

    QVariantMap info = m_musicModel->getMusicInfo(m_selectedIndex);
    return info["title"].toString();
}

QString MusicViewModel::selectedMusicArtist() const {
    if (!isValidIndex(m_selectedIndex))
        return QString();

    QVariantMap info = m_musicModel->getMusicInfo(m_selectedIndex);
    return info["artist"].toString();
}

QString MusicViewModel::selectedMusicUrl() const {
    if (!isValidIndex(m_selectedIndex))
        return QString();

    return m_musicModel->getAudioUrl(m_selectedIndex);
}

bool MusicViewModel::isMusicSelected() const {
    return isValidIndex(m_selectedIndex);
}

void MusicViewModel::selectMusic(int index) {
    if (!isValidIndex(index) && index != -1) {
        qWarning() << "无效的音乐索引:" << index;
        emit errorOccurred("无效的音乐选择");
        return;
    }

    setSelectedIndex(index);

    if (isValidIndex(index)) {
        QVariantMap info = m_musicModel->getMusicInfo(index);
        qDebug() << "选择了音乐:" << info["title"].toString() << "-" << info["artist"].toString();
        emit selectedMusicChanged();
    }
}

void MusicViewModel::toggleFavorite(int index) {
    if (!isValidIndex(index))
        return;

    m_musicModel->toggleFavorite(index);
    emit musicListUpdated();
}

// 修改现有的 playMusic 方法：
void MusicViewModel::playMusic(int index) {
    if (!isValidIndex(index))
        return;

    // 如果已经在播放，先停止
    if (m_isPlaying) {
        stopMusic();
    }

    m_currentPlayingIndex = index;
    m_isPlaying = true;

    // 更新模型中的播放状态
    m_musicModel->setPlaying(index);

    // 获取音频URL
    QString audioUrl = m_musicModel->getAudioUrl(index);

    // 发射信号，让QML中的MediaPlayer播放
    emit playAudioRequested(audioUrl);
    emit playingStateChanged();

    qDebug() << "播放音乐:" << index << "URL:" << audioUrl;
}

void MusicViewModel::searchMusic(const QString &keyword) {
    if (keyword.isEmpty()) {
        // 如果搜索关键词为空，可以显示所有音乐
        emit musicListUpdated();
        return;
    }

    QVariantList results = m_musicModel->searchMusic(keyword);
    qDebug() << "搜索关键词:" << keyword << "找到" << results.size() << "个结果";

    // 这里可以触发UI更新，比如显示搜索结果
    // 实际项目中可能需要一个独立的SearchModel
    emit musicListUpdated();
}

void MusicViewModel::loadMusic() {
    m_musicModel->loadLocalMusic();
    emit musicListUpdated();
    qDebug() << "音乐数据已加载，共" << m_musicModel->rowCount() << "首音乐";
}

void MusicViewModel::clearSelection() {
    setSelectedIndex(-1);
    stopMusic();  // 添加这行
    emit selectedMusicChanged();
    qDebug() << "已清除音乐选择";
}

QVariantMap MusicViewModel::getCurrentMusicInfo() const {
    if (!isValidIndex(m_selectedIndex))
        return QVariantMap();

    return m_musicModel->getMusicInfo(m_selectedIndex);
}

bool MusicViewModel::confirmSelection() {
    if (!isValidIndex(m_selectedIndex)) {
        emit errorOccurred("请先选择音乐");
        return false;
    }

    QVariantMap musicInfo = getCurrentMusicInfo();
    emit musicSelectedForEdit(musicInfo);

    qDebug() << "音乐选择已确认:" << musicInfo["title"].toString();
    return true;
}

void MusicViewModel::setSelectedIndex(int index) {
    if (m_selectedIndex == index)
        return;

    m_selectedIndex = index;
    emit selectedIndexChanged(index);
}

bool MusicViewModel::isValidIndex(int index) const {
    return index >= 0 && index < m_musicModel->rowCount();
}

// 添加新的方法实现：
void MusicViewModel::playCurrentMusic() {
    if (isValidIndex(m_selectedIndex)) {
        playMusic(m_selectedIndex);
    }
}

void MusicViewModel::pauseMusic() {
    m_isPlaying = false;
    emit playingStateChanged();
    qDebug() << "暂停音乐";
}

void MusicViewModel::stopMusic() {
    if (m_currentPlayingIndex >= 0) {
        m_musicModel->setPlaying(-1);  // 重置所有播放状态
    }

    m_isPlaying = false;
    m_currentPlayingIndex = -1;
    emit playingStateChanged();
    qDebug() << "停止音乐";
}

bool MusicViewModel::isPlaying() const {
    return m_isPlaying;
}

QString MusicViewModel::currentPlayingMusic() const {
    if (!isValidIndex(m_currentPlayingIndex))
        return QString();

    QVariantMap info = m_musicModel->getMusicInfo(m_currentPlayingIndex);
    return info["title"].toString();
}
