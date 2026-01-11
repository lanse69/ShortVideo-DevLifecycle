#include "videopublisher.h"
#include <QDebug>
#include <QFileInfo>

VideoPublisher* VideoPublisher::m_instance = nullptr;

VideoPublisher::VideoPublisher(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_uploadFile(nullptr)
    , m_isUploading(false)
    , m_uploadProgress(0) {

    // 从配置管理器获取端点地址
    ConfigManager &config = ConfigManager::instance();
    m_uploadEndpoint = config.getUploadEndpoint();
    m_apiServerUrl = config.getApiServerUrl();
}

void VideoPublisher::publishVideo(const QString &filePath, const QString &title, const QString &token) {
    // 检查token是否有效
    if (token.isEmpty()) {
        emit publishFailed("用户未登录，请先登录");
        return;
    }

    // 保存token
    m_authToken = token;

    // 检查文件是否存在
    if (!QFile::exists(filePath)) {
        emit publishFailed(QString("文件不存在: %1").arg(filePath));
        return;
    }

    // 重置状态
    cleanup();

    // 保存参数
    m_videoTitle = title;

    // 生成UUID
    m_videoUUID = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_videoUUID = m_videoUUID.left(8);

    // 更新状态
    m_isUploading = true;
    emit isUploadingChanged();

    updateStatus("开始上传视频...");
    updateProgress(0);

    // 开始上传
    uploadToMinIO(filePath);
}

void VideoPublisher::uploadToMinIO(const QString &filePath) {
    // 构造上传URL
    m_tempUploadUrl = QString("%1/temp/%2.mp4").arg(m_uploadEndpoint).arg(m_videoUUID);
    qDebug() << "[VideoPublisher]:m_tempUploadUrl:"<<m_tempUploadUrl;

    qDebug() << "[VideoPublisher]:filePath:"<<filePath;
    // 打开文件
    m_uploadFile = new QFile(filePath, this);
    if (!m_uploadFile->open(QIODevice::ReadOnly)) {
        qDebug() << "[VideoPublisher]:无法打开视频文件";
        updateStatus("无法打开视频文件");
        cleanup();
        emit publishFailed("无法打开视频文件");
        return;
    }

    // 创建PUT请求
    QNetworkRequest request{QUrl(m_tempUploadUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "video/mp4");

    // 【新增】显式设置 Content-Length，防止服务器因为不知道长度而直接重置连接
    request.setHeader(QNetworkRequest::ContentLengthHeader, m_uploadFile->size());

    // 发送请求
    m_currentReply = m_networkManager->put(request, m_uploadFile);

    // 连接信号
    connect(m_currentReply, &QNetworkReply::uploadProgress, [this](qint64 sent, qint64 total) {
        if (total > 0) {
            int progress = static_cast<int>((sent * 100) / total);
            updateProgress(progress);
        }
    });

    connect(m_currentReply, &QNetworkReply::finished, this, &VideoPublisher::onMinIOUploadFinished);
}

void VideoPublisher::onMinIOUploadFinished() {
    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;

    if (reply->error() == QNetworkReply::NoError) {
        updateProgress(100);
        updateStatus("视频上传成功，正在提交处理任务...");
        qDebug() << "[VideoPublisher]:视频上传成功 ";
        // 提交处理任务
        submitPublishTask();
    } else {
        QString error = QString("上传失败: %1").arg(reply->errorString());

        qDebug() << "[VideoPublisher]: "<<error;

        updateStatus(error);
        cleanup();
        emit publishFailed(error);
    }

    reply->deleteLater();
}

void VideoPublisher::submitPublishTask() {
    // 使用保存的token
    QString token = m_authToken;

    // 构造API请求URL
    QString apiUrl = QString("%1/api/video/publish").arg(m_apiServerUrl);

    // 创建请求体
    QJsonObject jsonBody;
    jsonBody["title"] = m_videoTitle;
    jsonBody["url"] = m_tempUploadUrl;

    QJsonDocument doc(jsonBody);
    QByteArray jsonData = doc.toJson();

    // 创建POST请求
    QNetworkRequest request{QUrl(apiUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());

    // 发送请求
    m_currentReply = m_networkManager->post(request, jsonData);
    connect(m_currentReply, &QNetworkReply::finished, this, &VideoPublisher::onTaskSubmitFinished);
}

void VideoPublisher::onTaskSubmitFinished() {
    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);

        QString videoId = m_videoUUID;
        if (!doc.isNull()) {
            QJsonObject json = doc.object();
            videoId = json["video_id"].toString(m_videoUUID);
        }

        updateStatus("发布成功！视频正在后台转码中...");
        cleanup();
        emit publishSuccess(videoId);
    } else {
        QString error = QString("任务提交失败: %1").arg(reply->errorString());
        updateStatus(error);
        cleanup();
        emit publishFailed(error);
    }

    reply->deleteLater();
}

void VideoPublisher::cancelUpload() {
    if (m_currentReply) {
        m_currentReply->abort();
    }
    cleanup();
    updateStatus("上传已取消");
}

void VideoPublisher::cleanup() {
    if (m_currentReply) {
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    if (m_uploadFile) {
        if (m_uploadFile->isOpen()) {
            m_uploadFile->close();
        }
        m_uploadFile->deleteLater();
        m_uploadFile = nullptr;
    }

    m_isUploading = false;
    emit isUploadingChanged();

    m_uploadProgress = 0;
    emit uploadProgressChanged();
}

void VideoPublisher::updateStatus(const QString &message) {
    m_statusMessage = message;
    emit statusMessageChanged();
}

void VideoPublisher::updateProgress(int progress) {
    m_uploadProgress = progress;
    emit uploadProgressChanged();

    if (progress < 100) {
        m_statusMessage = QString("上传中... %1%").arg(progress);
        emit statusMessageChanged();
    }
}
