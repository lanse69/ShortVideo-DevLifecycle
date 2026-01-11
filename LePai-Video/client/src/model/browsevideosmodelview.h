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
    Q_INVOKABLE void requestVideos();

signals:
    void videosLoaded(const QVariantList &videoList);
    void videosRequestFailed(QString m_errorMessage);


private:
    QString m_apiBaseUrl;
    QMap<QString, VideoModel> m_videoMap;
    bool m_isLoading;
    QString m_errorMessage;

    int m_nextOffset{};
    void handleVideosResponse(bool success, const QJsonObject &response);
    void parseVideoData(const QByteArray &data);
};
