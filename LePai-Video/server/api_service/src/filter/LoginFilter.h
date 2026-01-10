#pragma once

#include <drogon/HttpFilter.h>

class LoginFilter : public drogon::HttpFilter<LoginFilter>
{
public:
    LoginFilter() {}
    
    // 过滤器
    void doFilter(const drogon::HttpRequestPtr &req, drogon::FilterCallback &&fcb, drogon::FilterChainCallback &&ccb) override;
};