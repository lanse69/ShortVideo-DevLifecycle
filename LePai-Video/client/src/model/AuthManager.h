#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QQmlEngine>
#include "usermodel.h"

class AuthManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(UserModel* currentUser READ currentUser NOTIFY currentUserChanged)
    Q_PROPERTY(bool wasLogin READ wasLogin WRITE setwasLogin NOTIFY wasLoginChanged)

public:
    explicit AuthManager(QObject *parent = nullptr);

    Q_INVOKABLE void registerUser(const QString &username, const QString &password);
    Q_INVOKABLE void login(const QString &username, const QString &password);
    Q_INVOKABLE QString getToken() const;
    //Q_INVOKABLE void logout(); //登出方法

signals:
    void registrationSuccess();
    void registrationFailed(const QString &message);
    void loginSuccess();
    void loginFailed(const QString &message);
    void wasLoginChanged();

    void currentUserChanged();

private:
    QNetworkAccessManager *m_networkManager;
    QString m_apiBaseUrl;
    QString m_token; // 存储登录后的token
    UserModel *m_currentUser;  // 存储当前用户信息
    bool m_wasLogin = false;

    UserModel *currentUser() const { return m_currentUser; }
    bool wasLogin() { return m_wasLogin; }
    void setwasLogin(bool b) {m_wasLogin=b;}

};
