#pragma once

#include <string>
#include <memory>
#include <drogon/HttpClient.h>

namespace lepai {
namespace storage {

class MinioClient {
public:
    MinioClient(const std::string& endpoint, const std::string& accessKey, const std::string& secretKey);

    /**
     * @brief 上传文件到对象存储
     * @param bucket 存储桶名称
     * @param objectKey 对象键名
     * @param localPath 本地文件路径
     * @param publicBaseUrl CDN 基础地址 (用于生成返回的 URL)
     * @return std::string 成功返回完整 URL，失败返回空字符串
     */
    std::string uploadFile(const std::string& bucket, const std::string& objectKey, const std::string& localPath, const std::string& publicBaseUrl);

    // 删除文件
    bool removeFile(const std::string& bucket, const std::string& objectKey);

private:
    std::string endpoint_;
    std::string accessKey_;
    std::string secretKey_;

    std::shared_ptr<drogon::HttpClient> client_;
};

}
}