#include "videomodel.h"
#include <QJsonObject>
#include <QJsonDocument>

VideoModel::VideoModel()
    : m_duration(0), m_likeCount(0), m_isLiked(false) {
}

VideoModel::VideoModel(const QString& id,  const QString& title,
                       const QString& url, const QString& coverUrl, int duration,
                       long long likeCount,
                       const QString& authorName, const QString& authorAvatar, bool isLiked,bool isFollowed)
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
    , m_isLiked(isLiked)
    ,m_isFollowed(isFollowed)
{
}

VideoModel VideoModel::fromJson(const QJsonObject &json) {
    VideoModel video;
    // 按照服务端字段名解析
    video.m_id = json["id"].toString();
    video.m_title = json["title"].toString();
    video.m_url = json["url"].toString();
    video.m_coverUrl = json["cover_url"].toString();
    video.m_duration = json["duration"].toInt();
    video.m_likeCount = json["like_count"].toInt();
    video.m_authorName = json["author"].toString();
    video.m_authorAvatar = json["author_avatar"].toString();
    video.m_isLiked = json["is_liked"].toBool();
    video.m_isFollowed=json["is_followed"].toBool();
    qDebug() <<"fromJson m_isLiked:" <<video.m_isLiked;
    qDebug() <<"fromJson m_likeCount:" <<video.m_likeCount;
    return video;
}

QJsonObject VideoModel::toJson() const {
    QJsonObject json;
    json["id"] = m_id;
    json["title"] = m_title;
    json["url"] = m_url;
    json["cover_url"] = m_coverUrl;
    json["duration"] = m_duration;
    json["like_count"] = static_cast<qint64>(m_likeCount);
    json["author"] = m_authorName;
    json["author_avatar"] = m_authorAvatar;
    json["is_liked"] = m_isLiked;
    json["is_followed"] = m_isFollowed;
    return json;
}

QVariantMap VideoModel::toVariantMap() const {
    QVariantMap map;
    map["id"] = m_id;
    //map["userId"] = m_userId;
    map["title"] = m_title;
    map["url"] = m_url;
    map["coverUrl"] = m_coverUrl; 
    map["duration"] = m_duration;
    map["likeCount"] = m_likeCount; 
    //map["createdAt"] = m_createdAt;
    map["authorName"] = m_authorName;
    map["authorAvatar"] = m_authorAvatar;
    map["isLiked"] = m_isLiked;
    map["isFollowed"] = m_isFollowed;
    qDebug()<<"toVariantMap m_likeCount:" <<m_likeCount;
    qDebug()<<"toVariantMap m_isLiked:" <<m_isLiked;

    return map;
}
