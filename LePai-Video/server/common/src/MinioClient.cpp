#include "MinioClient.h"

#include <drogon/drogon.h>
#include <drogon/utils/Utilities.h>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace lepai {
namespace storage {

MinioClient::MinioClient(const std::string& endpoint, const std::string& accessKey, const std::string& secretKey)
    : endpoint_(endpoint), accessKey_(accessKey), secretKey_(secretKey) 
{
    // 初始化 HTTP 客户端
    std::string url = "http://" + endpoint;
    client_ = drogon::HttpClient::newHttpClient(url);
}

std::string MinioClient::uploadFile(const std::string& bucket, const std::string& objectKey, const std::string& localPath, const std::string& publicBaseUrl) 
{
    // 基础校验
    if (!std::filesystem::exists(localPath)) {
        LOG_ERROR << "File not found: " << localPath;
        return "";
    }

    // 读取文件内容到内存
    std::ifstream file(localPath, std::ios::binary);
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string fileContent = ss.str();

    // 构建 HTTP PUT 请求
    auto req = drogon::HttpRequest::newHttpRequest();
    // 路径: /bucket/objectKey
    req->setPath("/" + bucket + "/" + objectKey);
    req->setMethod(drogon::Put);
    req->setBody(fileContent);
    req->setContentTypeCode(drogon::CT_APPLICATION_OCTET_STREAM);

    // 设置认证 (Basic Auth: base64(user:pass))
    std::string authStr = accessKey_ + ":" + secretKey_;
    std::string authBase64 = drogon::utils::base64Encode(authStr);
    req->addHeader("Authorization", "Basic " + authBase64);

    // 发送请求 (同步等待)
    std::promise<bool> prom;
    auto future = prom.get_future();

    client_->sendRequest(req, [&](drogon::ReqResult result, const drogon::HttpResponsePtr &response) {
        if (result == drogon::ReqResult::Ok && response->getStatusCode() == 200) {
            prom.set_value(true);
        } else {
            if (response) {
                LOG_ERROR << "MinIO Upload Failed: " << response->getStatusCode() << " " << response->getBody();
            } else {
                LOG_ERROR << "MinIO Network Error";
            }
            prom.set_value(false);
        }
    });

    // 等待结果 (30秒超时)
    if (future.wait_for(std::chrono::seconds(30)) == std::future_status::ready) {
        bool success = future.get();
        if (success) {
            // 返回标准化的 CDN URL
            std::string finalUrl = publicBaseUrl;
            if (finalUrl.back() != '/') finalUrl += "/";
            finalUrl += bucket + "/" + objectKey;
            return finalUrl;
        }
    } else {
        LOG_ERROR << "MinIO Upload Timeout: " << objectKey;
    }

    return "";
}

}
}