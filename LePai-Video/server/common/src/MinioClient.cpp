#include "MinioClient.h"

#include <drogon/drogon.h>
#include <drogon/utils/Utilities.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <ctime>

namespace lepai {
namespace storage {

// SHA256 Hash -> Hex String
static std::string sha256Hex(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.c_str(), data.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// HMAC-SHA256
static std::string hmacSha256(const std::string& key, const std::string& data) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    HMAC(EVP_sha256(), key.c_str(), key.length(), 
         (unsigned char*)data.c_str(), data.length(), hash, &len);
    return std::string((char*)hash, len);
}

// 获取当前时间的 Date 和 Time 字符串
static void getTimes(std::string& dateStr, std::string& timeStr) {
    std::time_t now = std::time(nullptr);
    std::tm* gmt = std::gmtime(&now);
    
    char dateBuf[9]; // YYYYMMDD\0
    std::strftime(dateBuf, sizeof(dateBuf), "%Y%m%d", gmt);
    dateStr = dateBuf;

    char timeBuf[17]; // YYYYMMDDThhmmssZ\0
    std::strftime(timeBuf, sizeof(timeBuf), "%Y%m%dT%H%M%SZ", gmt);
    timeStr = timeBuf;
}

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

    // 读取文件内容
    std::ifstream file(localPath, std::ios::binary);
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string fileContent = ss.str();
    
    // 计算 Payload Hash
    std::string contentSha256 = sha256Hex(fileContent);

    // 时间参数
    std::string dateStr, timeStr;
    getTimes(dateStr, timeStr);
    
    // 路径和Host
    std::string uri = "/" + bucket + "/" + objectKey;
    std::string region = "us-east-1"; // MinIO 默认 Region
    std::string service = "s3";

    // HTTPMethod + '\n' + CanonicalURI + '\n' + CanonicalQueryString + '\n' + CanonicalHeaders + '\n' + SignedHeaders + '\n' + HashedPayload
    
    std::string canonicalHeaders = "host:" + endpoint_ + "\n" +
                                   "x-amz-content-sha256:" + contentSha256 + "\n" +
                                   "x-amz-date:" + timeStr + "\n";
    
    std::string signedHeaders = "host;x-amz-content-sha256;x-amz-date";

    std::string canonicalRequest = "PUT\n" + 
                                   uri + "\n" + 
                                   "" + "\n" + // 队列未空
                                   canonicalHeaders + "\n" + 
                                   signedHeaders + "\n" + 
                                   contentSha256;

    // 构造 String to Sign
    std::string algorithm = "AWS4-HMAC-SHA256";
    std::string scope = dateStr + "/" + region + "/" + service + "/aws4_request";
    std::string stringToSign = algorithm + "\n" +
                               timeStr + "\n" +
                               scope + "\n" +
                               sha256Hex(canonicalRequest);

    // 计算 Signature Key
    // kDate = HMAC("AWS4" + kSecret, Date)
    // kRegion = HMAC(kDate, Region)
    // kService = HMAC(kRegion, Service)
    // kSigning = HMAC(kService, "aws4_request")
    std::string kSecret = "AWS4" + secretKey_;
    std::string kDate = hmacSha256(kSecret, dateStr);
    std::string kRegion = hmacSha256(kDate, region);
    std::string kService = hmacSha256(kRegion, service);
    std::string kSigning = hmacSha256(kService, "aws4_request");

    // 计算 Signature
    std::string signatureRaw = hmacSha256(kSigning, stringToSign);
    
    // Hex encode signature
    std::stringstream sigSs;
    for(unsigned char c : signatureRaw) {
        sigSs << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    std::string signature = sigSs.str();

    // 构造 Authorization Header
    std::string authHeader = algorithm + " Credential=" + accessKey_ + "/" + scope + 
                             ", SignedHeaders=" + signedHeaders + 
                             ", Signature=" + signature;


    // 发送请求
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath(uri);
    req->setMethod(drogon::Put);
    req->setBody(fileContent);
    req->setContentTypeCode(drogon::CT_APPLICATION_OCTET_STREAM);

    // 设置必须的头
    req->addHeader("Host", endpoint_);
    req->addHeader("X-Amz-Date", timeStr);
    req->addHeader("X-Amz-Content-Sha256", contentSha256);
    req->addHeader("Authorization", authHeader);

    LOG_DEBUG << "Uploading to " << uri << " with Auth: " << authHeader;

    std::promise<bool> prom;
    auto future = prom.get_future();

    client_->sendRequest(req, [&](drogon::ReqResult result, const drogon::HttpResponsePtr &response) {
        if (result == drogon::ReqResult::Ok && (response->getStatusCode() == 200 || response->getStatusCode() == 204)) {
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

    if (future.wait_for(std::chrono::seconds(60)) == std::future_status::ready) {
        bool success = future.get();
        if (success) {
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

bool MinioClient::removeFile(const std::string& bucket, const std::string& objectKey) 
{
    // 准备时间
    std::string dateStr, timeStr;
    getTimes(dateStr, timeStr);

    // 空 Payload 的 SHA256
    std::string contentSha256 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

    // 路径
    std::string uri = "/" + bucket + "/" + objectKey;
    std::string region = "us-east-1";
    std::string service = "s3";

    // 构造 Canonical Request
    std::string canonicalHeaders = "host:" + endpoint_ + "\n" +
                                   "x-amz-content-sha256:" + contentSha256 + "\n" +
                                   "x-amz-date:" + timeStr + "\n";
    std::string signedHeaders = "host;x-amz-content-sha256;x-amz-date";

    std::string canonicalRequest = "DELETE\n" + 
                                   uri + "\n" + 
                                   "" + "\n" + 
                                   canonicalHeaders + "\n" + 
                                   signedHeaders + "\n" + 
                                   contentSha256;

    // 构造 String to Sign
    std::string algorithm = "AWS4-HMAC-SHA256";
    std::string scope = dateStr + "/" + region + "/" + service + "/aws4_request";
    std::string stringToSign = algorithm + "\n" +
                               timeStr + "\n" +
                               scope + "\n" +
                               sha256Hex(canonicalRequest);

    // 计算签名
    std::string kSecret = "AWS4" + secretKey_;
    std::string kDate = hmacSha256(kSecret, dateStr);
    std::string kRegion = hmacSha256(kDate, region);
    std::string kService = hmacSha256(kRegion, service);
    std::string kSigning = hmacSha256(kService, "aws4_request");
    
    std::string signatureRaw = hmacSha256(kSigning, stringToSign);
    std::stringstream sigSs;
    for(unsigned char c : signatureRaw) sigSs << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    std::string signature = sigSs.str();

    // Authorization Header
    std::string authHeader = algorithm + " Credential=" + accessKey_ + "/" + scope + 
                             ", SignedHeaders=" + signedHeaders + 
                             ", Signature=" + signature;

    // 发送请求
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath(uri);
    req->setMethod(drogon::Delete); // DELETE 方法
    req->addHeader("Host", endpoint_);
    req->addHeader("X-Amz-Date", timeStr);
    req->addHeader("X-Amz-Content-Sha256", contentSha256);
    req->addHeader("Authorization", authHeader);

    LOG_DEBUG << "Deleting " << uri;

    std::promise<bool> prom;
    auto future = prom.get_future();

    client_->sendRequest(req, [&](drogon::ReqResult result, const drogon::HttpResponsePtr &response) {
        // 204 No Content 删除成功
        if (result == drogon::ReqResult::Ok && (response->getStatusCode() == 204 || response->getStatusCode() == 200)) {
            prom.set_value(true);
        } else {
            if(response) LOG_ERROR << "Delete Failed: " << response->getStatusCode();
            prom.set_value(false);
        }
    });

    if (future.wait_for(std::chrono::seconds(10)) == std::future_status::ready) {
        return future.get();
    }
    return false;
}

}
}