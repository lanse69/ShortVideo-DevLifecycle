#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QVector>
#include <QString>
#include <QUrl>

// 音乐数据项
struct MusicItem {
    QString id;
    QString title;          // 歌曲标题
    QString artist;         // 歌手
    QString album;          // 专辑
    QString coverUrl;       // 封面URL
    QUrl audioUrl;          // 音频URL
    int duration;           // 时长（秒）
    bool isFavorite;        // 是否收藏
    bool isPlaying;         // 是否正在播放
    int playCount;          // 播放次数

    MusicItem() : duration(0), isFavorite(false), isPlaying(false), playCount(0) {}
};

// 音乐数据模型
class MusicModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum MusicRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        ArtistRole,
        AlbumRole,
        CoverUrlRole,
        AudioUrlRole,
        DurationRole,
        IsFavoriteRole,
        IsPlayingRole,
        PlayCountRole,
        FormattedDurationRole
    };

    explicit MusicModel(QObject *parent = nullptr);

    // QAbstractListModel 接口
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 数据操作方法
    Q_INVOKABLE void loadLocalMusic();  // 加载本地音乐（模拟服务端数据）
    Q_INVOKABLE void addMusic(const QString &title, const QString &artist,
                              const QString &audioUrl, int duration = 0);
    Q_INVOKABLE void removeMusic(int index);
    Q_INVOKABLE void toggleFavorite(int index);
    Q_INVOKABLE void setPlaying(int index);
    Q_INVOKABLE void incrementPlayCount(int index);

    // 获取音乐信息
    Q_INVOKABLE QVariantMap getMusicInfo(int index) const;
    Q_INVOKABLE QString getAudioUrl(int index) const;

    // 搜索和过滤
    Q_INVOKABLE QVariantList searchMusic(const QString &keyword);
    Q_INVOKABLE QVariantList getFavorites();

private:
    QVector<MusicItem> m_musicList;

    // 初始化本地模拟数据
    void initializeLocalMusic();
};

