#include "browsevideosmodelview.h"
#include "../ConfigManager.h"
#include "networkclient.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QUrlQuery>

BrowseVideosModelView::BrowseVideosModelView(QObject *parent)
    : QObject{parent}
{
    m_isLoading = false;
}
void BrowseVideosModelView::requestVideos()
{
    if (m_isLoading) {
        qDebug() << "[BrowseVideos] 已经在加载中，跳过请求";
        return;
    }

    m_isLoading = true;
    m_errorMessage.clear();

    // 使用 NetworkClient 发送请求
    NetworkClient::instance().requestVideos(m_nextOffset, 3,
        [this](bool success, QJsonObject response) {
            this->handleVideosResponse(success, response);
        });
}

void BrowseVideosModelView::handleVideosResponse(bool success, const QJsonObject &response)
{
    m_isLoading = false;

    if (!success) {
        QString message = response["message"].toString("未知错误");
        m_errorMessage = "请求失败: " + message;
        qDebug() << "[BrowseVideos] 请求失败:" << m_errorMessage;
        emit videosRequestFailed(m_errorMessage);
        return;
    }

    QJsonDocument doc(response);
    parseVideoData(doc.toJson());
}


void BrowseVideosModelView::parseVideoData(const QByteArray &data) {
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        m_errorMessage = "JSON解析错误: " + parseError.errorString();
        m_isLoading = false;

        qDebug() << "[BrowseVideos] JSON解析错误:" << parseError.errorString();

        emit videosRequestFailed(m_errorMessage);
        return;
    }

    if (!doc.isObject()) {
        m_errorMessage = "响应格式不正确，应为JSON对象";
        m_isLoading = false;

        qDebug() << "[BrowseVideos]" << m_errorMessage;

        emit videosRequestFailed(m_errorMessage);
        return;
    }

    QJsonObject rootObj = doc.object();

    // 检查响应码
    int code = rootObj["code"].toInt(0);
    if (code != 200) {
        QString message = rootObj["message"].toString("未知错误");
        m_errorMessage = QString("服务端错误 (%1): %2").arg(code).arg(message);
        m_isLoading = false;

        qDebug() << "[BrowseVideos] 服务端返回错误:" << m_errorMessage;

        emit videosRequestFailed(m_errorMessage);
        return;
    }

    // 解析 data 数组
    if (!rootObj.contains("data") || !rootObj["data"].isArray()) {
        m_errorMessage = "响应格式不正确，缺少data字段或不是数组";
        m_isLoading = false;

        qDebug() << "[BrowseVideos]" << m_errorMessage;

        emit videosRequestFailed(m_errorMessage);
        return;
    }

    QJsonArray dataArray = rootObj["data"].toArray();
    qDebug() << "[BrowseVideos] 收到视频数组，数量:" << dataArray.size();

    // 构建临时列表
    QVariantList videoVariantList{};

    for (const QJsonValue &value : dataArray) {
        if (value.isObject()) {
            VideoModel video = VideoModel::fromJson(value.toObject());//*******
            if (!video.getVideoId().isEmpty()) {
                m_videoMap.insert(video.getVideoId(), video);
                videoVariantList.append(video.toVariantMap());
            }
        }
    }

    // 检查是否有下一页（next_offset）
    if (rootObj.contains("next_offset")) {
        int nextOffset = rootObj["next_offset"].toInt(-1);
        qDebug() << "[BrowseVideos] 下一页偏移量:" << nextOffset;
        // 这里保存 nextOffset 用于分页加载更多
        m_nextOffset = nextOffset;
    }

    m_isLoading = false;

    if (videoVariantList.isEmpty()) {
        m_errorMessage = "没有获取到视频数据";
        qDebug() << "[BrowseVideos]" << m_errorMessage;

        emit videosRequestFailed(m_errorMessage);
    } else {
        qDebug() << "[BrowseVideos] 成功加载" << videoVariantList.size() << "个视频";
        emit videosLoaded(videoVariantList);
    }
}
