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
               const QString& authorName, const QString& authorAvatar, bool isLiked,bool isFollowed);

    static VideoModel fromJson(const QJsonObject &json);
    QJsonObject toJson() const;
    QVariantMap toVariantMap() const;

    QString getVideoId() { return m_id; }

    bool isLiked() const { return m_isLiked; }
    void setLiked(bool liked) { m_isLiked = liked;  }

    int likeCount() const { return m_likeCount; }
    void setLikeCount(int count) { m_likeCount = count; }

private:
    QString m_id; 
    //QString m_userId;   
    QString m_title;
    QString m_url;
    QString m_coverUrl;  
    int m_duration;
    long long m_likeCount;
    //QString m_createdAt;
    QString m_authorName;  
    QString m_authorAvatar; 
    bool m_isLiked;
    bool m_isFollowed;
};
