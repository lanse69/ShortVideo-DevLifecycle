#include "networkclient.h"
#include "../ConfigManager.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QUrlQuery>
#include <QFile>
#include <QString>

NetworkClient::NetworkClient(QObject *parent):QObject(parent) {
    m_networkManager = new QNetworkAccessManager(this);
    m_apiBaseUrl = ConfigManager::instance().getApiServerUrl();
    m_cdnServerUrl=ConfigManager::instance().getCdnServerUrl();
    m_uploadEndpoint=ConfigManager::instance().getUploadEndpoint();
}

NetworkClient &NetworkClient::instance()
{
    static NetworkClient instance;
    return instance;
}

void NetworkClient::sendRegisterRequest(const QString &username, const QString &password, std::function<void(bool re,QString message)> callBack)
{
    QUrl url(m_apiBaseUrl + "/api/user/register");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hashedPassword = hashBytes.toHex();
    QJsonObject json;
    json["username"] = username;
    json["password"] = hashedPassword;
    QByteArray data = QJsonDocument(json).toJson();
    QNetworkReply *reply = m_networkManager->post(request, data);

    qDebug() << "Sending login request to:" << url.toString();

    connect(reply, &QNetworkReply::finished, [reply,callBack]() {
        if (reply->error() == QNetworkReply::NoError) {
            QString succStr = "注册成功";
            callBack(true,succStr);
        } else {
            QString errStr = reply->errorString();
            QByteArray respData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(respData);
            if (!doc.isNull() && doc.object().contains("details")) {
                errStr = doc.object()["details"].toString();
            } else if (!doc.isNull() && doc.object().contains("message")) {
                errStr = doc.object()["message"].toString();
            }
            callBack(false,errStr);
        }
        reply->deleteLater();
    });
}

void NetworkClient::sendLoginRequest(const QString &username, const QString &password,
                                     std::function<void(bool success, QJsonObject response)> callBack)
{
    QUrl url(m_apiBaseUrl + "/api/user/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hashedPassword = QString::fromLatin1(hashBytes.toHex());

    QJsonObject json;
    json["username"] = username;
    json["password"] = hashedPassword;  // 注意：这里应该是已经哈希过的密码
    QByteArray data = QJsonDocument(json).toJson();

    qDebug() << "Sending login request to:" << url.toString();

    QNetworkReply *reply = m_networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, [reply, callBack]() {
        QJsonObject responseObj;
        bool success = false;

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray respData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(respData);

            if (!doc.isNull()) {
                responseObj = doc.object();
                int code = responseObj["code"].toInt();
                success = (code == 200);
            } else {
                responseObj["message"] = "响应格式错误";
            }
        } else {
            QString errStr = reply->errorString();
            QByteArray respData = reply->readAll();

            // 尝试从响应中提取错误信息
            if (!respData.isEmpty()) {
                QJsonDocument doc = QJsonDocument::fromJson(respData);
                if (!doc.isNull()) {
                    QJsonObject obj = doc.object();
                    if (obj.contains("message")) {
                        errStr = obj["message"].toString();
                    }
                }
            }
            responseObj["message"] = errStr;
        }

        if (callBack) {
            callBack(success, responseObj);
        }

        reply->deleteLater();
    });
}

void NetworkClient::requestVideos(int offset, int limit,
                                  std::function<void(bool success, QJsonObject response)> callback)
{
    QUrl url(m_apiBaseUrl + "/api/feed/discovery");
    QUrlQuery query;
    query.addQueryItem("limit", QString::number(limit));
    query.addQueryItem("offset", QString::number(offset));
    url.setQuery(query);

    QNetworkRequest request(url);
    qDebug() << "Sending video request to:" << url.toString();

    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, [reply, callback]() {
        QJsonObject responseObj;
        bool success = false;

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray respData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(respData);

            if (!doc.isNull()) {
                responseObj = doc.object();
                int code = responseObj["code"].toInt();
                success = (code == 200);
            } else {
                responseObj["message"] = "响应格式错误";
            }
        } else {
            QString errStr = reply->errorString();
            QByteArray respData = reply->readAll();

            // 尝试从响应中提取错误信息
            if (!respData.isEmpty()) {
                QJsonDocument doc = QJsonDocument::fromJson(respData);
                if (!doc.isNull()) {
                    QJsonObject obj = doc.object();
                    if (obj.contains("message")) {
                        errStr = obj["message"].toString();
                    } else if (obj.contains("error")) {
                        errStr = obj["error"].toString();
                    } else if (obj.contains("details")) {
                        errStr = obj["details"].toString();
                    }
                }
            }
            responseObj["message"] = errStr;
        }

        if (callback) {
            callback(success, responseObj);
        }

        reply->deleteLater();
    });
}

void NetworkClient::uploadVideoFile(const QString &filePath, const QString &uuid,
                                    std::function<void(bool success, QString error, QString uploadUrl)> callback)
{
    // 构造上传 URL
    QString uploadUrl = QString("%1/temp/%2.mp4").arg(m_uploadEndpoint).arg(uuid);
    qDebug() << "[NetworkClient] 上传URL:" << uploadUrl;

    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        QString error = "无法打开视频文件";
        qDebug() << "[NetworkClient] 上传失败:" << error;
        file->deleteLater();
        if (callback) callback(false, error, "");
        return;
    }

    QNetworkRequest request{QUrl(uploadUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "video/mp4");
    request.setHeader(QNetworkRequest::ContentLengthHeader, file->size());

    QNetworkReply *reply = m_networkManager->put(request, file);
    file->setParent(reply);

    connect(reply, &QNetworkReply::finished, [reply, callback, uploadUrl]() {
        bool success = (reply->error() == QNetworkReply::NoError);
        QString error = success ? "" : reply->errorString();

        if (callback) {
            callback(success, error, uploadUrl);
        }

        reply->deleteLater();
    });
}

void NetworkClient::submitPublishTask(const QString &title, const QString &videoUrl, const QString &token,
                                      std::function<void(bool success, QJsonObject response)> callback)
{
    QString apiUrl = QString("%1/api/video/publish").arg(m_apiBaseUrl);
    QNetworkRequest request{QUrl(apiUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());

    QJsonObject jsonBody;
    jsonBody["title"] = title;
    jsonBody["url"] = videoUrl;

    QByteArray jsonData = QJsonDocument(jsonBody).toJson();

    QNetworkReply *reply = m_networkManager->post(request, jsonData);

    connect(reply, &QNetworkReply::finished, [reply, callback]() {
        QJsonObject responseObj;
        bool success = false;

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray respData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(respData);

            if (!doc.isNull()) {
                responseObj = doc.object();
                int code = responseObj["code"].toInt();
                success = (code == 200);
            } else {
                responseObj["message"] = "响应格式错误";
            }
        } else {
            QString errStr = reply->errorString();
            QByteArray respData = reply->readAll();

            if (!respData.isEmpty()) {
                QJsonDocument doc = QJsonDocument::fromJson(respData);
                if (!doc.isNull()) {
                    QJsonObject obj = doc.object();
                    if (obj.contains("message")) {
                        errStr = obj["message"].toString();
                    }
                }
            }
            responseObj["message"] = errStr;
        }

        if (callback) {
            callback(success, responseObj);
        }

        reply->deleteLater();
    });
}
