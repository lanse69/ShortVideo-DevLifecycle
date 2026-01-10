#include "MinioClient.h"

#include <drogon/drogon.h>
#include <cstdlib>
#include <sstream>
#include <filesystem>

namespace lepai {
namespace storage {

MinioClient::MinioClient(const std::string& endpoint, const std::string& accessKey, const std::string& secretKey)
    : endpoint_(endpoint), accessKey_(accessKey), secretKey_(secretKey) {}

std::string MinioClient::uploadFile(const std::string& bucket, const std::string& objectKey, const std::string& localPath, const std::string& publicBaseUrl) 
{
    // 基础校验
    if (!std::filesystem::exists(localPath)) {
        LOG_ERROR << "File not found: " << localPath;
        return "";
    }

    // 构建 MinIO 内部上传 API 地址
    std::string uploadUrl = "http://" + endpoint_ + "/" + bucket + "/" + objectKey;
    
    // 使用 curl 进行 PUT 上传
    // -s: Silent mode
    // -f: Fail silently (return error code) on HTTP errors
    // --create-dirs: 并没有用，因为是 PUT 到 URL
    std::stringstream cmd;
    cmd << "curl -s -f " 
        << "-u " << accessKey_ << ":" << secretKey_ << " "
        << "-T \"" << localPath << "\" "
        << "\"" << uploadUrl << "\"";
        
    LOG_DEBUG << "Uploading to MinIO: " << objectKey;
    
    int ret = std::system(cmd.str().c_str());

    if (ret != 0) {
        LOG_ERROR << "Failed to upload file via MinIO CLI. Return Code: " << ret;
        return "";
    }

    // 返回标准化的 CDN URL
    // 格式: http://cdn_host:port/bucket/objectKey
    std::string finalUrl = publicBaseUrl;
    if (finalUrl.back() != '/') finalUrl += "/";
    finalUrl += bucket + "/" + objectKey;

    return finalUrl;
}

}
}