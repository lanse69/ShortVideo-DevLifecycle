#include "usermodel.h"

#include <QJsonObject>
#include <QDebug>

UserModel::UserModel(QObject *parent) : QObject(parent)
{
}

void UserModel::updateFromJson(const QJsonObject &json)
{
    bool changed = false;

    // 更新 ID
    if (json.contains("id") && json["id"].isString()) {
        QString newId = json["id"].toString();
        if (m_id != newId) {
            m_id = newId;
            changed = true;
        }
    }

    // 更新用户名
    if (json.contains("username") && json["username"].isString()) {
        QString newUsername = json["username"].toString();
        if (m_username != newUsername) {
            m_username = newUsername;
            changed = true;
        }
    }

    // 更新头像URL
    if (json.contains("avatar_url") && json["avatar_url"].isString()) {
        QString newAvatarUrl = json["avatar_url"].toString();
        if (m_avatarUrl != newAvatarUrl) {
            m_avatarUrl = newAvatarUrl;
            changed = true;
        }
    }

    // 更新创建时间
    if (json.contains("created_at") && json["created_at"].isString()) {
        QString newCreatedAt = json["created_at"].toString();
        if (m_createdAt != newCreatedAt) {
            m_createdAt = newCreatedAt;
            changed = true;
        }
    }

    // 更新关注数
    if (json.contains("following_count") && json["following_count"].isDouble()) {
        int newCount = json["following_count"].toInt();
        if (m_followingCount != newCount) {
            m_followingCount = newCount;
            changed = true;
        }
    }

    // 更新粉丝数
    if (json.contains("follower_count") && json["follower_count"].isDouble()) {
        int newCount = json["follower_count"].toInt();
        if (m_followerCount != newCount) {
            m_followerCount = newCount;
            changed = true;
        }
    }

    // 更新是否关注
    if (json.contains("is_followed") && json["is_followed"].isBool()) {
        bool newIsFollowed = json["is_followed"].toBool();
        if (m_isFollowed != newIsFollowed) {
            m_isFollowed = newIsFollowed;
            changed = true;
        }
    }

    // 如果有变化，发出信号
    if (changed) {
        emit dataChanged();
    }
}

void UserModel::clear()
{
    bool changed = false;

    if (!m_id.isEmpty()) {
        m_id.clear();
        changed = true;
    }

    if (!m_username.isEmpty()) {
        m_username.clear();
        changed = true;
    }

    if (!m_avatarUrl.isEmpty()) {
        m_avatarUrl.clear();
        changed = true;
    }

    if (!m_createdAt.isEmpty()) {
        m_createdAt.clear();
        changed = true;
    }

    if (m_followingCount != 0) {
        m_followingCount = 0;
        changed = true;
    }

    if (m_followerCount != 0) {
        m_followerCount = 0;
        changed = true;
    }

    if (m_isFollowed) {
        m_isFollowed = false;
        changed = true;
    }

    if (changed) {
        emit dataChanged();
    }
}
