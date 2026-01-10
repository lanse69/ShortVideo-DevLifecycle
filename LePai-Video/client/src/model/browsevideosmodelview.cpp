#include "browsevideosmodelview.h"
#include "../ConfigManager.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrlQuery>

BrowseVideosModelView::BrowseVideosModelView(QObject *parent)
    : QObject{parent}
{
    m_networkManager = new QNetworkAccessManager(this);
    m_apiBaseUrl = ConfigManager::instance().getApiServerUrl();
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

    // 构造请求URL，使用查询参数
    QUrl url(m_apiBaseUrl + "/api/feed/discovery");
    QUrlQuery query;
    query.addQueryItem("limit", "3");  // 可以调整为合适的值
    query.addQueryItem("offset", QString::number(m_nextOffset));
    url.setQuery(query);

    qDebug() << "[BrowseVideos] 请求视频列表URL:" << url.toString();

    QNetworkRequest request(url);

    // 注意：服务端是GET请求，不是POST
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished,
            this, &BrowseVideosModelView::onNetworkReplyVideosFinished);
}

void BrowseVideosModelView::onNetworkReplyVideosFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    //在槽函数中，sender() 函数返回发射信号的对象的指针：
    if (!reply) {
        return;
    }

    //判断传输数据是否出错
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "[BrowseVideos] 收到响应数据，大小:" << data.size();
        parseVideoData(data);
    } else {
        QString errorStr = reply->errorString();
        QByteArray respData = reply->readAll();

        qDebug() << "[BrowseVideos] 网络请求失败:" << errorStr;
        qDebug() << "[BrowseVideos] 响应数据:" << respData;

        // 尝试从响应中提取错误信息
        if (!respData.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(respData);
            if (!doc.isNull() && doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("message")) {
                    errorStr = obj["message"].toString();
                } else if (obj.contains("error")) {
                    errorStr = obj["error"].toString();
                } else if (obj.contains("details")) {
                    errorStr = obj["details"].toString();
                }
            }
        }

        m_errorMessage = "网络请求失败: " + errorStr;
        m_isLoading = false;
        emit videosRequestFailed(m_errorMessage);
    }
    reply->deleteLater();
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
        // 你可以在这里保存 nextOffset 用于分页加载更多
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
