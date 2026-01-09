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
    METHOD_LIST_END

    FeedController();

    void getDiscoveryFeed(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);

private:
    std::shared_ptr<lepai::service::RecommendationService> recommendService;

    // 从 Header 解析 UserId，如果 Token 无效返回空字符串
    void tryGetUserId(const drogon::HttpRequestPtr& req, std::function<void(std::string)> callback);
};