#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QQmlEngine>

class AuthManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit AuthManager(QObject *parent = nullptr);

    Q_INVOKABLE void registerUser(const QString &username, const QString &password);

signals:
    void registrationSuccess();
    void registrationFailed(const QString &message);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_apiBaseUrl;
};