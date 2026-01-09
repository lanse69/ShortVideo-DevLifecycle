#pragma once

#include <string>

namespace lepai {
namespace worker {
namespace storage {

class MinioClient {
public:
    MinioClient(const std::string& endpoint, const std::string& accessKey, const std::string& secretKey);

    // 上传本地文件到 MinIO
    // bucket: 桶名
    // objectName: 对象键
    // localPath: 本地文件路径
    // 返回: 完整的访问 URL (经过 CDN 代理的地址)
    std::string uploadFile(const std::string& bucket, const std::string& objectName, const std::string& localPath, const std::string& publicBaseUrl);

private:
    std::string endpoint_;
    std::string accessKey_;
    std::string secretKey_;
};

}
}
}