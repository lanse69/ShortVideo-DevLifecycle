#pragma once

#include <drogon/HttpController.h>

#include "../service/RecommendationService.h"
#include "../repository/SessionRepository.h"

class FeedController : public drogon::HttpController<FeedController>
{
public:
    METHOD_LIST_BEGIN
    // GET /api/feed/discovery?limit=10&offset=0
    ADD_METHOD_TO(FeedController::getDiscoveryFeed, "/api/feed/discovery", drogon::Get);
    // GET /api/feed/following?limit=10&offset=0
    ADD_METHOD_TO(FeedController::getFollowingFeed, "/api/feed/following", drogon::Get, "LoginFilter");
    // GET /api/feed/likes?user_id={target_id}&limit=10&offset=0
    ADD_METHOD_TO(FeedController::getLikedFeed, "/api/feed/likes", drogon::Get);
    // GET /api/feed/user?user_id={target_id}&limit=10&offset=0
    ADD_METHOD_TO(FeedController::getUserUploadFeed, "/api/feed/user", drogon::Get);
    METHOD_LIST_END

    FeedController();

    void getDiscoveryFeed(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    void getFollowingFeed(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    void getLikedFeed(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    void getUserUploadFeed(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    
private:
    std::shared_ptr<lepai::service::RecommendationService> recommendService;

    // 从 Header 解析 UserId，如果 Token 无效返回空字符串
    void tryGetUserId(const drogon::HttpRequestPtr& req, std::function<void(std::string)> callback);
};