#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QQmlEngine>
#include <QJSEngine>
#include <QUuid>
#include "AuthManager.h"
#include "ConfigManager.h"

class VideoPublisher : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isUploading READ isUploading NOTIFY isUploadingChanged)
    Q_PROPERTY(int uploadProgress READ uploadProgress NOTIFY uploadProgressChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit VideoPublisher(QObject *parent = nullptr);

    // QML可调用的方法
    Q_INVOKABLE void publishVideo(const QString &filePath, const QString &title,const QString &token);

    // 获取属性值
    bool isUploading() const { return m_isUploading; }
    int uploadProgress() const { return m_uploadProgress; }
    QString statusMessage() const { return m_statusMessage; }

signals:
    void isUploadingChanged();
    void uploadProgressChanged();
    void statusMessageChanged();
    void publishSuccess(const QString &videoId);
    void publishFailed(const QString &errorMessage);

private:
    void uploadToMinIO(const QString &filePath);
    void submitPublishTask();
    void cleanup();
    void updateStatus(const QString &message);
    void updateProgress(int progress);

    static VideoPublisher* m_instance;

    QFile *m_uploadFile;

    QString m_videoUUID;
    QString m_tempUploadUrl;
    QString m_videoTitle;

    QString m_authToken;

    // 属性
    bool m_isUploading;
    int m_uploadProgress;
    QString m_statusMessage;
};
