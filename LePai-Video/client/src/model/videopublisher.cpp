#include "videopublisher.h"
#include "networkclient.h"
#include <QDebug>
#include <QFileInfo>

VideoPublisher* VideoPublisher::m_instance = nullptr;

VideoPublisher::VideoPublisher(QObject *parent)
    : QObject(parent)
    , m_uploadFile(nullptr)
    , m_isUploading(false)
    , m_uploadProgress(0) {
}

void VideoPublisher::uploadToMinIO(const QString &filePath) {
    NetworkClient::instance().uploadVideoFile(filePath, m_videoUUID,
        [this](bool success, QString error,QString uploadUrl) {
            if (success) {
                m_tempUploadUrl = uploadUrl; // 保存上传后的 URL
                qDebug() << "[VideoPublisher]: 视频上传成功，URL:" << uploadUrl;
                updateProgress(100);
                updateStatus("视频上传成功，正在提交处理任务...");
                submitPublishTask();
            } else {
                QString errorMsg = QString("上传失败: %1").arg(error);
                qDebug() << "[VideoPublisher]: " << errorMsg;
                updateStatus(errorMsg);
                cleanup();
                emit publishFailed(errorMsg);
            }
      });
}

void VideoPublisher::submitPublishTask() {
    NetworkClient::instance().submitPublishTask(m_videoTitle, m_tempUploadUrl, m_authToken,
        [this](bool success, QJsonObject response) {
            if (success) {
                QString videoId = response["video_id"].toString(m_videoUUID);
                updateStatus("发布成功！视频正在后台转码中...");
                cleanup();
                emit publishSuccess(videoId);
            } else {
                QString error = QString("任务提交失败: %1").arg(response["message"].toString());
                updateStatus(error);
                cleanup();
                emit publishFailed(error);
            }
        });
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

void VideoPublisher::cleanup() {
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
