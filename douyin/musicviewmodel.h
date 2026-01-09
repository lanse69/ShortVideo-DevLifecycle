#pragma once

#include <QObject>
#include <QAbstractListModel>
#include "musicmodel.h"

class MusicViewModel : public QObject {
    Q_OBJECT

    // View绑定的属性
    Q_PROPERTY(QAbstractListModel* musicList READ musicList CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(QString selectedMusicTitle READ selectedMusicTitle NOTIFY selectedMusicChanged)
    Q_PROPERTY(QString selectedMusicArtist READ selectedMusicArtist NOTIFY selectedMusicChanged)
    Q_PROPERTY(QString selectedMusicUrl READ selectedMusicUrl NOTIFY selectedMusicChanged)
    Q_PROPERTY(bool isMusicSelected READ isMusicSelected NOTIFY selectedMusicChanged)

    // 在 Q_PROPERTY 部分添加：
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playingStateChanged)
    Q_PROPERTY(QString currentPlayingMusic READ currentPlayingMusic NOTIFY playingStateChanged)

public:
    explicit MusicViewModel(QObject *parent = nullptr);

    // 属性访问器
    QAbstractListModel* musicList() const;
    int selectedIndex() const;
    QString selectedMusicTitle() const;
    QString selectedMusicArtist() const;
    QString selectedMusicUrl() const;
    bool isMusicSelected() const;
    bool isPlaying() const;
    QString currentPlayingMusic() const;

    // View调用的方法
    Q_INVOKABLE void selectMusic(int index);
    Q_INVOKABLE void toggleFavorite(int index);
    Q_INVOKABLE void playMusic(int index);
    Q_INVOKABLE void searchMusic(const QString &keyword);
    Q_INVOKABLE void loadMusic();
    Q_INVOKABLE void clearSelection();

    // 在 public 部分添加：
    Q_INVOKABLE void playCurrentMusic();
    Q_INVOKABLE void pauseMusic();
    Q_INVOKABLE void stopMusic();

    // 用于编辑页面的方法
    Q_INVOKABLE QVariantMap getCurrentMusicInfo() const;
    Q_INVOKABLE bool confirmSelection();

public slots:
    void setSelectedIndex(int index);

signals:
    void selectedIndexChanged(int index);
    void selectedMusicChanged();
    void musicListUpdated();
    void musicPlayRequested(const QString &musicUrl, const QString &title);
    void musicSelectedForEdit(const QVariantMap &musicInfo);
    void errorOccurred(const QString &message);
    void playingStateChanged();
    void playAudioRequested(const QString &audioUrl);

private:
    MusicModel* m_musicModel;
    int m_selectedIndex;

    bool m_isPlaying;
    int m_currentPlayingIndex;

    // 验证索引是否有效
    bool isValidIndex(int index) const;
};
