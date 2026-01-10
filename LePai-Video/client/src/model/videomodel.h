#pragma once

#include <QString>
#include <QJsonObject>
#include <QVariantMap>

class VideoModel {
public:
    VideoModel();
    VideoModel(const QString& id,  const QString& title,
               const QString& url, const QString& coverUrl, int duration,
               long long likeCount,
               const QString& authorName, const QString& authorAvatar, bool isLiked = false);

    static VideoModel fromJson(const QJsonObject &json);
    QJsonObject toJson() const;
    QVariantMap toVariantMap() const;

    QString getVideoId() { return m_id; }

private:
    QString m_id;            // 对应服务端 id
    //QString m_userId;        // 对应服务端 userId
    QString m_title;
    QString m_url;
    QString m_coverUrl;      // 对应服务端 coverUrl
    int m_duration;
    long long m_likeCount;
    //QString m_createdAt;
    QString m_authorName;    // 对应服务端 author
    QString m_authorAvatar;  // 对应服务端 author_avatar
    bool m_isLiked;
    bool m_isFollowed;
};
