#include "AuthManager.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QDebug>

#include "../ConfigManager.h"

AuthManager::AuthManager(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);

    // 获取 API 地址
    m_apiBaseUrl = ConfigManager::instance().getApiServerUrl();
    
    if (m_apiBaseUrl.isEmpty()) {
        qWarning() << "API Base URL is empty! Check config.json";
    }
}

void AuthManager::registerUser(const QString &username, const QString &password)
{
    QUrl url(m_apiBaseUrl + "/api/user/register");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 密码哈希 (SHA256)
    QByteArray hashBytes = QCryptographicHash::hash(
        password.toUtf8(), 
        QCryptographicHash::Sha256
    );
    QString hashedPassword = hashBytes.toHex();

    QJsonObject json;
    json["username"] = username;
    json["password"] = hashedPassword;
    
    QByteArray data = QJsonDocument(json).toJson();

    qDebug() << "Sending register request to:" << url.toString();

    QNetworkReply *reply = m_networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit registrationSuccess();
        } else {
            QString errStr = reply->errorString();
            QByteArray respData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(respData);
            if (!doc.isNull() && doc.object().contains("details")) {
                errStr = doc.object()["details"].toString();
            } else if (!doc.isNull() && doc.object().contains("message")) {
                 errStr = doc.object()["message"].toString();
            }
            emit registrationFailed(errStr);
        }
        reply->deleteLater();
    });
}