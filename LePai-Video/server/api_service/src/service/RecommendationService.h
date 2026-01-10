#pragma once

#include <vector>
#include <memory>

#include "../repository/VideoRepository.h"
#include "../repository/UserRepository.h"

namespace lepai {
namespace service {

class RecommendationService {
public:
    RecommendationService();

    // 推荐页
    // userId: 为空则为游客，不为空则需检查点赞状态
    void getDiscoveryFeed(const std::string& userId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback);

    // 关注页
    void getFollowingFeed(const std::string& userId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback);

    // 点赞页
    // targetUserId: 查看谁的点赞列表
    // currentUserId: 当前登录用户
    void getLikedFeed(const std::string& targetUserId, const std::string& currentUserId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback);

    // 用户作品页
    // targetUserId: 查看谁的作品
    // currentUserId: 当前登录用户
    void getUserUploadFeed(const std::string& targetUserId, const std::string& currentUserId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback);

private:
    std::shared_ptr<lepai::repository::VideoRepository> videoRepo;
    std::shared_ptr<lepai::repository::UserRepository> userRepo;

    // 如果不走缓存或缓存未命中，查库
    void fetchFromDb(const std::string& userId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback);
    
    // 填充个性化信息
    void enrichUserData(const std::string& userId, std::vector<lepai::entity::Video> videos, lepai::repository::VideoRepository::FeedCallback callback);
};

}
}