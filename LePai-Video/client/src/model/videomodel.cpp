#include "videomodel.h"
#include <QJsonObject>
#include <QJsonDocument>

VideoModel::VideoModel()
    : m_duration(0), m_likeCount(0), m_isLiked(false) {
}

VideoModel::VideoModel(const QString& id,  const QString& title,
                       const QString& url, const QString& coverUrl, int duration,
                       long long likeCount,
                       const QString& authorName, const QString& authorAvatar, bool isLiked)
    : m_id(id)
    //, m_userId(userId)
    , m_title(title)
    , m_url(url)
    , m_coverUrl(coverUrl)
    , m_duration(duration)
    , m_likeCount(likeCount)
    //, m_createdAt(createdAt)
    , m_authorName(authorName)
    , m_authorAvatar(authorAvatar)
    , m_isLiked(isLiked) {
}

VideoModel VideoModel::fromJson(const QJsonObject &json) {
    VideoModel video;
    // 按照服务端字段名解析
    video.m_id = json["id"].toString();
    video.m_title = json["title"].toString();
    video.m_url = json["url"].toString();
    video.m_coverUrl = json["cover_url"].toString();  // 注意字段名映射
    video.m_duration = json["duration"].toInt();
    video.m_likeCount = json["like_count"].toInt();  // JSON 中是整数
    video.m_authorName = json["author"].toString();  // 服务端返回的是 author
    video.m_authorAvatar = json["author_avatar"].toString();  // 服务端返回的是 author_avatar
    video.m_isLiked = json["is_liked"].toBool();  // 注意字段名映射

    // 注意：服务端返回的JSON中没有userId和createdAt字段
    // 这些字段在服务端的Video结构体中有，但toJson()方法没有包含它们
    // 所以这里我们留空或设置为默认值
    // video.m_userId = "";
    // video.m_createdAt = "";

    return video;
}

QJsonObject VideoModel::toJson() const {
    QJsonObject json;
    json["id"] = m_id;
    json["title"] = m_title;
    json["url"] = m_url;
    json["cover_url"] = m_coverUrl;
    json["duration"] = m_duration;
    json["like_count"] = static_cast<qint64>(m_likeCount);  // 转换为JSON兼容的整型
    json["author"] = m_authorName;
    json["author_avatar"] = m_authorAvatar;
    json["is_liked"] = m_isLiked;
    return json;
}

QVariantMap VideoModel::toVariantMap() const {
    QVariantMap map;
    map["id"] = m_id;
    //map["userId"] = m_userId;
    map["title"] = m_title;
    map["url"] = m_url;
    map["coverUrl"] = m_coverUrl;      // 注意字段名映射
    map["duration"] = m_duration;
    map["likeCount"] = m_likeCount;    // QML会自动处理long long到整型的转换
    //map["createdAt"] = m_createdAt;
    map["authorName"] = m_authorName;  // 注意字段名映射
    map["authorAvatar"] = m_authorAvatar;
    map["isLiked"] = m_isLiked;
    return map;
}
