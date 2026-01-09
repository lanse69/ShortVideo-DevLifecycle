#include "MinioClient.h"

#include <drogon/drogon.h>
#include <cstdlib>
#include <sstream>

namespace lepai {
namespace worker {
namespace storage {

MinioClient::MinioClient(const std::string& endpoint, const std::string& accessKey, const std::string& secretKey)
    : endpoint_(endpoint), accessKey_(accessKey), secretKey_(secretKey) {}

std::string MinioClient::uploadFile(const std::string& bucket, const std::string& objectName, const std::string& localPath, const std::string& publicBaseUrl) {
    // 构建 MinIO 内部上传 API 地址
    std::string uploadUrl = "http://" + endpoint_ + "/" + bucket + "/" + objectName;
    
    // 构建 CURL 命令
    std::stringstream cmd;
    cmd << "curl -s -f " // -s: 静默模式, -f: HTTP错误时返回非零状态码
        << "-u " << accessKey_ << ":" << secretKey_ << " "
        << "-T \"" << localPath << "\" "
        << "\"" << uploadUrl << "\"";
        
    LOG_DEBUG << "Uploading to MinIO: " << uploadUrl;
    int ret = std::system(cmd.str().c_str());

    if (ret != 0) {
        LOG_ERROR << "Failed to upload file to MinIO. Ret Code: " << ret;
        return "";
    }

    // 返回 CDN 格式的公开 URL
    return publicBaseUrl + "/" + bucket + "/" + objectName;
}

}
}
}