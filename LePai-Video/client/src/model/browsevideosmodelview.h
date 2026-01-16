#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QNetworkAccessManager>
#include <QJsonObject>

#include "videomodel.h"

class BrowseVideosModelView : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit BrowseVideosModelView(QObject *parent = nullptr);
    Q_INVOKABLE void requestVideos(const QString &token = "");
    Q_INVOKABLE void requestFollowingVideos(const QString &token = "");

    Q_INVOKABLE void likeVideo(const QString &videoId, bool action, const QString &token);
    Q_INVOKABLE void followUser(const QString &authorId, bool action, const QString &token);

signals:
    void videosLoaded(const QVariantList &videoList);
    void videosRequestFailed(QString m_errorMessage);

    void likeStatusChanged(const QString &videoId, bool isLiked, int likeCount);
    void likeFailed(const QString &videoId, const QString &errorMessage);

    void followStatusChanged(const QString &authorId, bool isFollowed);
    void followFailed(const QString &authorId, const QString &errorMessage);

private:
    QMap<QString, VideoModel> m_videoMap;
    bool m_isLoading;
    QString m_errorMessage;
    int m_nextOffset{};

    void handleVideosResponse(bool success, const QJsonObject &response);
    void parseVideoData(const QByteArray &data);
};
