#include "AuthManager.h"
#include "networkclient.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QSettings>
#include <QDebug>

#include "../ConfigManager.h"

AuthManager::AuthManager(QObject *parent) : QObject(parent)
{
    m_currentUser = new UserModel(this);

    // 从本地读取 Token
    QSettings settings;
    QString savedToken = settings.value("auth/token").toString();

    if (!savedToken.isEmpty()) {
        m_token = savedToken;
        m_wasLogin = true;
        qDebug() << "[AuthManager] 检测到本地 Token，自动登录成功";
    } else {
        qDebug() << "[AuthManager] 未检测到本地 Token";
    }
}

void AuthManager::registerUser(const QString &username, const QString &password)
{
    NetworkClient::instance().sendRegisterRequest(username, password, [this](bool re, QString message) {
        if (re) {
            emit registrationSuccess();
        } else {
            emit registrationFailed(message);
        }
    });
}

void AuthManager::login(const QString &username, const QString &password)
{
    NetworkClient::instance().sendLoginRequest(username, password,
       [this](bool success, QJsonObject response) {
           if (success) {
               // 登录成功
               QString token = response["token"].toString();
               m_token = token;

               // 保存 Token 到本地
               QSettings settings;
               settings.setValue("auth/token", token);

               // 更新用户信息
               if (response.contains("user") && response["user"].isObject()) {
                   QJsonObject userData = response["user"].toObject();
                   m_currentUser->updateFromJson(userData);
               }

               // 更新登录状态
               m_wasLogin = true;
               emit loginSuccess();
               emit wasLoginChanged();
           } else {
               // 登录失败
               QString errorMsg = response["message"].toString("登录失败");
               emit loginFailed(errorMsg);
               qDebug() << "登录失败:" << errorMsg;
           }
       });
}

void AuthManager::logout()
{
    m_token.clear();
    m_wasLogin = false;

    // 清除本地保存的 token
    QSettings settings;
    settings.remove("auth/token");

    // 清除用户信息
    if (m_currentUser) {
        m_currentUser->clear();
    }

    emit wasLoginChanged();
}

QString AuthManager::getToken() const {
    return m_token;
}
