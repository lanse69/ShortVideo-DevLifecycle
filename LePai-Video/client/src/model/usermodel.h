#pragma once

#include <QString>
#include <QObject>
class UserModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id NOTIFY dataChanged)
    Q_PROPERTY(QString username READ username NOTIFY dataChanged)
    Q_PROPERTY(QString avatarUrl READ avatarUrl NOTIFY dataChanged)
    Q_PROPERTY(QString createdAt READ createdAt NOTIFY dataChanged)
    Q_PROPERTY(int followingCount READ followingCount NOTIFY dataChanged)
    Q_PROPERTY(int followerCount READ followerCount NOTIFY dataChanged)
    Q_PROPERTY(bool isFollowed READ isFollowed NOTIFY dataChanged)
public:
    explicit UserModel(QObject *parent = nullptr);

    // 简单的 getter 方法
    QString id() const { return m_id; }
    QString username() const { return m_username; }
    QString avatarUrl() const { return m_avatarUrl; }
    QString createdAt() const { return m_createdAt; }
    int followingCount() const { return m_followingCount; }
    int followerCount() const { return m_followerCount; }
    bool isFollowed() const { return m_isFollowed; }

    // 主要功能：从 JSON 更新数据
    void updateFromJson(const QJsonObject &json);

    // 清空数据
    void clear();

signals:
    void dataChanged();

private:
    QString m_id;
    QString m_username;
    QString m_avatarUrl;
    QString m_createdAt;
    int m_followingCount = 0;
    int m_followerCount = 0;
    bool m_isFollowed = false;
};
