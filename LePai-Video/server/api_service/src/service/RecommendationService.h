#pragma once

#include <vector>
#include <memory>

#include "../repository/VideoRepository.h"

namespace lepai {
namespace service {

class RecommendationService {
public:
    RecommendationService();

    // userId: 为空则为游客，不为空则需检查点赞状态
    void getDiscoveryFeed(const std::string& userId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback);

private:
    std::shared_ptr<lepai::repository::VideoRepository> videoRepo;

    // 如果不走缓存或缓存未命中，查库
    void fetchFromDb(const std::string& userId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback);
    
    // 填充个性化信息
    void enrichUserData(const std::string& userId, std::vector<lepai::entity::Video> videos, lepai::repository::VideoRepository::FeedCallback callback);
};

}
}