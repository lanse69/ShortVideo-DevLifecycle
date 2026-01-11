#pragma once
#include <functional>
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>

class NetworkClient:QObject
{
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = nullptr);
    static NetworkClient& instance();

    void sendRegisterRequest(const QString &username, const QString &password, std::function<void(bool re, QString message)> callBack);
    void sendLoginRequest(const QString &username, const QString &password,
                          std::function<void(bool success, QJsonObject response)> callBack);
    void requestVideos(int offset, int limit,
                       std::function<void(bool success, QJsonObject response)> callback);

    void uploadVideoFile(const QString &filePath, const QString &uuid,
                          std::function<void(bool success, QString error, QString uploadUrl)> callback);
    void submitPublishTask(const QString &title, const QString &videoUrl, const QString &token,
                           std::function<void(bool success, QJsonObject response)> callback);
    void likeVideo(const QString &videoId, bool action, const QString &token,
                   std::function<void(bool success, int likeCount, QString error)> callback);

    QString m_apiBaseUrl;
    QString m_cdnServerUrl;
    QString m_uploadEndpoint;

    QNetworkAccessManager* m_networkManager;
};
