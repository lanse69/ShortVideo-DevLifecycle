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

    m_currentUser = new UserModel(this);
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

void AuthManager::login(const QString &username, const QString &password)
{
    QUrl url(m_apiBaseUrl + "/api/user/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 注意：需要与注册时使用相同的哈希方式
    // 如果注册时使用了SHA256哈希，登录时也应该使用相同的哈希
    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hashedPassword = hashBytes.toHex();

    QJsonObject json;
    json["username"] = username;
    json["password"] = hashedPassword; // 使用哈希后的密码

    QByteArray data = QJsonDocument(json).toJson();

    qDebug() << "Sending login request to:" << url.toString();

    QNetworkReply *reply = m_networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray respData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(respData);
            QJsonObject obj = doc.object();

            int code = obj["code"].toInt();
            if (code == 200) {
                // 登录成功
                QString token = obj["token"].toString();
                QJsonObject userData;
                // 保存token
                m_token = token;

                // 更新用户信息
                if (obj.contains("user") && obj["user"].isObject()) {
                    userData = obj["user"].toObject();
                    m_currentUser->updateFromJson(userData);
                }
                // 可选：将token保存到本地存储（如QSettings）
                // QSettings settings;
                // settings.setValue("auth/token", token);
                emit loginSuccess();
            } else {
                // 登录失败
                QString message = obj["message"].toString();
                emit loginFailed(message);
            }
        } else {
            // 网络错误
            QString errStr = reply->errorString();
            QByteArray respData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(respData);
            if (!doc.isNull()) {
                QJsonObject obj = doc.object();
                if (obj.contains("message")) {
                    errStr = obj["message"].toString();
                }
            }
            emit loginFailed(errStr);
        }
        reply->deleteLater();
    });
}

QString AuthManager::getToken() const {
    return m_token;
}
