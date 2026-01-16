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

void BrowseVideosModelView::requestVideos(const QString &token)
{
    if (m_isLoading) {
        qDebug() << "[BrowseVideos] 已经在加载中，跳过请求";
        return;
    }

    m_isLoading = true;
    m_errorMessage.clear();

    NetworkClient::instance().requestVideos(m_nextOffset, 3, token,
                                            [this](bool success, QJsonObject response) {
                                                this->handleVideosResponse(success, response);
                                            });
}

void BrowseVideosModelView::requestFollowingVideos(const QString &token)
{
    if (m_isLoading) {
        qDebug() << "[BrowseVideos] 已经在加载中，跳过请求";
        return;
    }

    m_isLoading = true;
    m_errorMessage.clear();

    NetworkClient::instance().requestFollowingVideos(m_nextOffset, 3, token,
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
            VideoModel video = VideoModel::fromJson(value.toObject());
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

void BrowseVideosModelView::likeVideo(const QString &videoId, bool action, const QString &token)
{
    // 直接检查传入的token
    if (token.isEmpty()) {
        emit likeFailed(videoId, "用户未登录");
        return;
    }

    // 立即更新本地状态
    if (m_videoMap.contains(videoId)) {
        VideoModel &video = m_videoMap[videoId];
        bool wasLiked = video.isLiked();
        int currentLikes = video.likeCount();

        // 更新本地状态
        video.setLiked(action);

        // 计算点赞数变化
        if (action && !wasLiked) {
            video.setLikeCount(currentLikes + 1);
        } else if (!action && wasLiked) {
            video.setLikeCount(currentLikes - 1);
        }

        // 发射信号通知UI更新
        emit likeStatusChanged(videoId, action, video.likeCount());
    }

    // 发送网络请求
    NetworkClient::instance().likeVideo(videoId, action, token,
        [this, videoId, action](bool success, int likeCount, QString error) {
            if (success) {
                // 使用服务端返回的准确数据更新本地状态
                if (m_videoMap.contains(videoId)) {
                    VideoModel &video = m_videoMap[videoId];
                    video.setLikeCount(likeCount);
                    video.setLiked(action);

                    emit likeStatusChanged(videoId, action, likeCount);
                }
                qDebug() << "[BrowseVideos] 点赞成功，视频:" << videoId
                         << "点赞数:" << likeCount;
            } else {
                // 网络请求失败，回滚到之前的状态
                if (m_videoMap.contains(videoId)) {
                    VideoModel &video = m_videoMap[videoId];
                    bool wasLiked = !action;
                    video.setLiked(wasLiked);

                    // 回滚点赞数
                    if (action) {
                        video.setLikeCount(video.likeCount() - 1);
                    } else {
                        video.setLikeCount(video.likeCount() + 1);
                    }

                    // 发射信号通知UI回滚
                    emit likeStatusChanged(videoId, wasLiked, video.likeCount());
                }

                // 发射失败信号
                emit likeFailed(videoId, error);
                qDebug() << "[BrowseVideos] 点赞失败，视频:" << videoId
                         << "错误:" << error;
            }
        });
}


void BrowseVideosModelView::followUser(const QString &authorId, bool action, const QString &token)
{
    if (token.isEmpty()) {
        emit followFailed(authorId, "用户未登录");
        return;
    }

    // 更新所有该作者的视频的 isFollowed 状态
    bool hasUpdated = false;
    for (auto it = m_videoMap.begin(); it != m_videoMap.end(); ++it) {
        if (it.value().getAuthorId() == authorId) {
            it.value().setFollowed(action);
            hasUpdated = true;
        }
    }

    if (hasUpdated) {
        emit followStatusChanged(authorId, action);
    }

    // 发送网络请求
    NetworkClient::instance().followUser(authorId, action, token,
         [this, authorId, action](bool success, QString error) {
             if (!success) {
                 // 网络请求失败，回滚到之前的状态
                 bool rollbackAction = !action;
                 for (auto it = m_videoMap.begin(); it != m_videoMap.end(); ++it) {
                     if (it.value().getAuthorId() == authorId) {
                         it.value().setFollowed(rollbackAction);
                     }
                 }
                 emit followStatusChanged(authorId, rollbackAction);
                 emit followFailed(authorId, error);
                 qDebug() << "[BrowseVideos] 关注失败，作者:" << authorId << "错误:" << error;
             }
         });
}
